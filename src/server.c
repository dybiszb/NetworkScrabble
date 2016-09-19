#include "../include/server.h"

/*
 * When SIGINT is catched, this variable will tell main loop to stop.
 */
volatile sig_atomic_t g_doWork = 1;

/*
 * When SIGINT is catched, this variable will tell main loop to stop.
 */
volatile sig_atomic_t g_reset = 1;

/*
 * When SIGCHLD is received, wait_for_client() must be repeated.
 */
volatile sig_atomic_t g_sigChld = 0;

/*
 * Core of the server.
 */
int main(void)
{
    int s, s2, pid, queueSemaphore, waitingPlayerSocketId;
    int waitForChildren;
    PlayerInfo* waitingPlayerSocketAdd = NULL;
    struct sockaddr_in local, remote;
	
	/* Waiting player socket in shared memory */
	shared_mem_init(&waitingPlayerSocketId,sizeof(int),'E');
	waitingPlayerSocketAdd = (PlayerInfo*)shared_mem_attach(waitingPlayerSocketId);
	waitingPlayerSocketAdd->status = NO_PLAYER;
	
	/* Queue semaphore */
	semaphore_init(&queueSemaphore, 'E', 1);

	/* Server socket */
	tcp_socket_init_unix(&s, &local);
	tcp_socket_bind(&s, &local);
	tcp_socket_listen(&s, INCOMMING_CONN);

	/* Set signals handlers */
	if(sethandler(sigint_handler,SIGINT)) ERR("main() : sethandler SIGINT:");
	if(sethandler(sigchld_handler,SIGCHLD)) ERR("main() : sethandler SIGCHLD:");
	
    while(g_doWork) {
        printf("[Server] Waiting for a clients...\n");
		if(INTERRUPTED == tcp_wait_for_client(&s2, s, &remote))
		{
			if(g_sigChld == 1)
			{
				g_sigChld = 0;
				continue;
			}
			else break;
		}
        printf("[Server] Someone has connected.\n");

		switch(pid = fork())
		{
			case 0:
				if(sethandler(sigint_handler,SIGINT)) ERR("main() : sethandler SIGINT:");
				handle_client(s2, queueSemaphore, waitingPlayerSocketAdd);
				printf("Client with pid: %d disconnected.\n", getpid());
				exit(EXIT_SUCCESS);
			default:
				if(pid < 0) ERR("main() : fork() ");
		}
        
    }
		
	/* Wait for children. */
	waitForChildren = 1;
	while(waitForChildren){
		pid=wait(NULL);
		if(pid<0)
			switch (errno){
				case ECHILD:
					waitForChildren = 0;
					break;
				case EINTR:
					continue;
				default:
					ERR("main() : wait ");
			}
	}

	printf("MAIN %d - cleaning up.\n",getpid());
	semaphore_remove(queueSemaphore);
	shared_mem_detach((char*)waitingPlayerSocketAdd);
	shared_mem_delete(waitingPlayerSocketId);
	
	/* Close and unlink socket */
	if(-1 == close(s)) printf("close EROOR!\n");

	exit(EXIT_SUCCESS);
}


void sigint_handler(int sig) {
	g_doWork = 0;
}

void sigchld_handler(int sig) {
	g_sigChld = 1;
	pid_t pid;
	for(;;){
		pid=waitpid(0, NULL, WNOHANG);
		if(pid==0) return;
		if(pid<=0) {
			if(errno==ECHILD) return;
			perror("waitpid:");
			exit(EXIT_FAILURE);
		}
	}
}

void handle_client(int clientDescriptor, int queueSemaphore, PlayerInfo* queue)
{
	while(1)
	{
		packet* msg = malloc(sizeof(packet));
		game* scrabbleGameAdd = NULL;
		int scrabbleGameId, gameSemId, playerType;
		char logName[100];

		if(msg == NULL)
		{
			ERR("handle_client() : malloc ");
		}

		/* ----- LOCK QUEUE SEMAPHORE ----- */		
		semaphore_lock(queueSemaphore, 0, 0);
		printf("[CLIENT %d] in charge of queue's Shared memory %d\n", clientDescriptor, queueSemaphore);		
		
		switch(queue->status)
		{
			case NO_PLAYER:
				/* Create game  in shared memory for future play */
				init_game(&scrabbleGameId);
				scrabbleGameAdd = (game*) shared_mem_attach(scrabbleGameId);
				*scrabbleGameAdd = scrabble_game_new_game();
								
				/* Create semaphores for the game flow */
				semaphore_init(&gameSemId,getpid(), 3);
				
				/* Create the log file's name.*/
				sprintf(logName, "game%d", (int) getpid());	
				
				/* Inform client about changes */
				send_msg(clientDescriptor, msg, NO_PLAYER);
				
				/* Save info for future player */
				write_to_queue(queue, WAITING_PLAYER, scrabbleGameId, gameSemId, logName);

				/* ----- UNLOCK QUEUE SEMAPHORE ----- */
				semaphore_unlock(queueSemaphore,0,0);
				
				/* Player 1 moves first */
				playerType = FIRST;		
				semaphore_lock(gameSemId, /* Player: */ 2 , 0); 
				
				/* Wait for another player to come */
				check_for_another_client(queue, clientDescriptor, queueSemaphore, gameSemId, scrabbleGameId);
				break;
				
			case WAITING_PLAYER:
				/* Retrieve infomration saved by first player. */
				read_from_queue(queue, &scrabbleGameId, &gameSemId, logName);
				
				/* Indicate that waiting queue is empty. */
				write_to_queue(queue, NO_PLAYER, NONE, NONE, "");
				
				/* ----- UNLOCK QUEUE SEMAPHORE ----- */
				semaphore_unlock(queueSemaphore,0,0);	

				/* ----- LOCK GAME SEMAPHORE ----- */
				semaphore_lock(gameSemId, 0,0);

				/* Attach to the shared memory address. */
				scrabbleGameAdd = (game*) shared_mem_attach(scrabbleGameId);
				
				/* Inform game that second player has connected. */
				scrabbleGameAdd->status = CONNECTED;

				/* ----- UNLOCK GAME SEMAPHORE ----- */
				semaphore_unlock(gameSemId, 0, 0);

				/* Player 2 - moves second */		
				playerType = SECOND;
				
				/* Since this player moves second, send empty game board and tiles*/
				write_to_packet(msg, INFO, NONE, NONE, NONE, 0 , 0, playerType, scrabbleGameAdd->gameBoard);
				tcp_socket_send_packet(clientDescriptor, msg);
				break;
		}

		printf("[Client %d] Game begins.\n", clientDescriptor);
		
		switch(main_game_loop(clientDescriptor, gameSemId, scrabbleGameId, logName, playerType))
		{
			case PLAY_AGAIN:
				local_cleanup(clientDescriptor, msg, scrabbleGameAdd, &scrabbleGameId, &gameSemId);
				break;
			case EXIT:
				local_cleanup(clientDescriptor, msg, scrabbleGameAdd, &scrabbleGameId, &gameSemId);
				close_socket(clientDescriptor);
				return;
		}
	}
}

void close_socket(int clientDescriptor)
{
		packet* msg = malloc(sizeof(packet));
		if(msg == NULL)
		{
			ERR("close_socket() : malloc ");
		}
		/* Close socket */
		msg->msg = EXIT;
		if ( NO_CLIENT == tcp_socket_send_packet(clientDescriptor,msg))
			printf("[CLIENT %d]No client on the other side.\n", clientDescriptor);

		printf("[CLIENT %d]CLEANUP - sending message.\n", clientDescriptor);
		if ( -1 == close(clientDescriptor)) printf("socket close error.");
		printf("[CLIENT %d]CLEANUP - soc cleared.\n", clientDescriptor);
		free(msg);
}

void local_cleanup(int clientDescriptor, packet* msg, game* scrabbleGameAdd, int* scrabbleGameId, int* gameSemId)
{
		/* Clean up */
		printf("[CLIENT %d]CLEANUP\n", clientDescriptor);
		
		/* Remove messagess structure. */
		free(msg);
		printf("[CLIENT %d]CLEANUP - msg's cleared.\n", clientDescriptor);
		
		/* Remove game's shared memory. */
        shared_mem_detach((char*)scrabbleGameAdd);
        shared_mem_delete(*scrabbleGameId);
        printf("[CLIENT %d]CLEANUP - shm's cleared.\n", clientDescriptor);
        
        /* Remove semaphores. */
        semaphore_remove(*gameSemId);
		printf("[CLIENT %d]CLEANUP - sem's cleared.\n", clientDescriptor);
}


void init_game(int* gameId)
{
	shared_mem_init(gameId, sizeof(game), (char) getpid());		
}

void send_msg(int fd, packet* msg, int type)
{
	msg->msg = type;
	tcp_socket_send_packet(fd, msg);
}

void write_to_queue(PlayerInfo* queue, int status, int gameSharedMemory, int gameSemaphore, char* logName)
{
	queue->status 				= status;
	queue->gameSharedMemory 	= gameSharedMemory;
	queue->gameSemaphore 		= gameSemaphore;
	strcpy(queue->logName, logName);
}

void read_from_queue(PlayerInfo* queue, int* gameSharedMemory, int* gameSemaphore, char* logName)
{
	*gameSharedMemory 			= queue->gameSharedMemory;
	*gameSemaphore 				= queue->gameSemaphore;
	strcpy(logName, queue->logName);
}

void write_to_packet(packet* msg,int message, char letter, int xCoord, int yCoord, int p1Points, int p2Points, int playerType,char board[5][5])
{
	int u,v;
	
	msg->msg 					= message;
	msg->letter 				= letter;
	msg->x_coord 				= xCoord;
	msg->y_coord 				= yCoord;
	msg->p1Points 				= p1Points;
	msg->p2Points 				= p2Points;
	msg->playerType 			= playerType;
		
	for(u = 0; u < 5; u++)
		for(v = 0; v <5; v++)
			msg->currentBoard[u][v] = board[u][v];
}

int check_for_another_client(PlayerInfo* queue, int clientDescriptor, int queueSemaphore, int gameSemaphore, int scrabbleGameId)
{
	packet* msg = malloc(sizeof(packet));
	game* scrabbleGameAdd = (game*) shared_mem_attach(scrabbleGameId);

	if(msg == NULL)
	{
		ERR("check_for_another_client() : malloc ");
	}

	while(g_doWork)
	{

		/* Check if client is still connected to the process. */
		if(recv(clientDescriptor, msg, sizeof(msg), MSG_DONTWAIT) == 0)
		{
			/* Remove data from the queue. */
			semaphore_lock(queueSemaphore, 0, 0);
			write_to_queue(queue, NO_PLAYER, NONE, NONE, NULL);
			semaphore_unlock(queueSemaphore, 0, 0);
    		printf("[CLIENT %d] disconnected\n", clientDescriptor);
			return LOST_CONNECTION;
				
		}

		/* Check if someone else connected to the game */
		semaphore_lock(gameSemaphore, 0, 0);

		if(scrabbleGameAdd->status == CONNECTED) 
		{
			/* Inform client that the game is about to start. */
			msg->msg = PAIR_MATCH;
			tcp_socket_send_packet(clientDescriptor, msg);
			semaphore_unlock(gameSemaphore, 0, 0);
			break;
		}
		semaphore_unlock(gameSemaphore, 0, 0);
	}

	free(msg);
	shared_mem_detach((char*)scrabbleGameAdd);
	
	return CONNECTED;
}

int main_game_loop(int clientDescriptor, int gameSemId, int gameShmId, char* logName, int playerType)
{
	packet* msg = malloc(sizeof(packet));
	game* scrabbleGameAdd = (game*) shared_mem_attach(gameShmId);
	int u, v;
	char playerTiles[5] = {'x', 'x', 'x', 'x', 'x'};
	int response;

	if(msg == NULL)
	{
		ERR("main_game_loop() : malloc");
	}

	/* Main game loop */
	while(g_doWork)
	{
		/* Mechanism for switching players at each turn */
		if(playerType == FIRST  && semaphore_lock(gameSemId, 1, 0) == INTERRUPTED) break;
		if(playerType == SECOND && semaphore_lock(gameSemId, 2, 0) == INTERRUPTED) break;

		/* If we are out of the tiles - game is ended */
		response = game_end_check(scrabbleGameAdd->movesLeft, playerType, gameSemId, clientDescriptor,
								  scrabbleGameAdd->p1Points, scrabbleGameAdd->p2Points, logName);
		if(PLAY_AGAIN == response) return PLAY_AGAIN;
		else if (INTERRUPTED == response || LOST_CONNECTION == response) break;

		/* Check if second player is still in game. */
		if(scrabbleGameAdd->status == NO_PLAYER) 
		{
			msg->msg = CHOICE_STAGE;
			tcp_socket_send_packet(clientDescriptor, msg);

			response = tcp_socket_read_packet(clientDescriptor,msg);
			if(LOST_CONNECTION == response || INTERRUPTED == response)    // <--- ?!??! pozmieniac
			{
				printf("[Client %d] Lost connection\n", clientDescriptor);
				log_summary(logName,scrabbleGameAdd->p1Points, scrabbleGameAdd->p2Points, "UNRESOLVED");
				break;
				
			}
			if(msg->msg == PLAY_AGAIN)
			{
				printf("[Client %d] Decided to play again.\n", clientDescriptor);
				printf("[Client %d] Adding to the queue...\n", clientDescriptor);
				return PLAY_AGAIN;
			}		
		}
		
		printf("[Client %d] in charge of game semaphore\n", clientDescriptor);
		
		/* Request new move from player */
		msg->msg 				= REQUEST_MOVE;
		msg->letter 			= NONE;
		msg->x_coord 			= NONE;
		msg->y_coord 			= NONE;
		msg->p1Points 			= scrabbleGameAdd->p1Points;
		msg->p2Points 			= scrabbleGameAdd->p2Points;
		msg->playerType 		= playerType;
		
		/* Game board info. */
		for(u = 0; u < 5; u++)
			for(v = 0; v <5; v++)
				msg->currentBoard[u][v] = scrabbleGameAdd->gameBoard[u][v];
		
		/* Tiles info. */
		for(u = 0; u < 5; u++)
			if(playerTiles[u] == 'x' && scrabbleGameAdd->movesLeft > 8)
			{ 
				char c = scrabble_game_get_random_tile(scrabbleGameAdd->avTiles);
				msg->tiles[u]  = c;
				playerTiles[u] = c;
			}
			else if(playerTiles[u] == 'x') msg->tiles[u] = 'x';

		printf("[Client %d] Tiles_sent:     %c %c %c %c %c\n", clientDescriptor, msg->tiles[0],
									msg->tiles[1], msg->tiles[2], msg->tiles[3],msg->tiles[4]);	
		printf	("[Client %d] P1 points: %d P2 points: %d\n",clientDescriptor, msg->p1Points, msg->p2Points);
		tcp_socket_send_packet(clientDescriptor, msg);
		
		/* Get new move. */
		response = tcp_socket_read_packet(clientDescriptor, msg);
		if(INTERRUPTED == response) 
		{
			msg->msg = EXIT;
			tcp_socket_send_packet(clientDescriptor, msg);
			log_summary(logName,scrabbleGameAdd->p1Points, scrabbleGameAdd->p2Points, "UNRESOLVED");
			break;
		}
		else if(LOST_CONNECTION == response) 
		{
			printf("[Client %d] Lost connection\n", clientDescriptor);
			// At this point one of the client has been disconnected
			// Another one must have a chance to decide, whether he wants
			// to play again or not. 
			// Although it is a process of disconnected client, we need to 
			// put in the shared memory 'note' about current situation and
			// pass the semaphore to the second client
			scrabbleGameAdd->status = NO_PLAYER;
			switch(playerType)
			{
			case FIRST:
				semaphore_unlock(gameSemId, 2, 0);
				break;
			case SECOND:
				semaphore_unlock(gameSemId, 1, 0);
				break;
			}
			log_summary(logName,scrabbleGameAdd->p1Points, scrabbleGameAdd->p2Points, "UNRESOLVED");
			free(msg);
			exit(EXIT_SUCCESS);
		};
		
		if(msg->msg == MOVE_DATA)
		{
			printf("[Client %d] New letter: %c\n", clientDescriptor,msg->letter); 
			printf("[Client %d] Coordinates: (%d,%d)\n", clientDescriptor, msg->x_coord, msg->y_coord);
			printf("[Client %d] Tiles_returned: %c %c %c %c %c\n", clientDescriptor, msg->tiles[0],
									msg->tiles[1], msg->tiles[2], msg->tiles[3],msg->tiles[4]);

			/* Log to file */
			log_entry(logName,msg->letter, msg->x_coord, msg->y_coord,
					  msg->p1Points, msg->p2Points);

									
			/* Update shared memory */
			scrabbleGameAdd->gameBoard[msg->x_coord][msg->y_coord] = msg->letter;
			scrabbleGameAdd->p1Points = msg->p1Points;
			scrabbleGameAdd->p2Points = msg->p2Points;
			
			/* Delete used tile */
			for(u = 0; u < 5; u++)
				if(playerTiles[u] == msg->letter)
				{
					playerTiles[u] = 'x';
					break;
				}
			printf("[Client %d] Tiles_current:  %c %c %c %c %c\n", clientDescriptor, playerTiles[0],
						playerTiles[1],playerTiles[2],playerTiles[3],playerTiles[4]);

			/* Decrease moves to control game flow */
			scrabbleGameAdd->movesLeft -= 1;
			printf("[Client %d] MovesLeft: %d.\n",clientDescriptor, scrabbleGameAdd->movesLeft);
		}
		else
			printf("[Client %d] Error in packet sent from client.", clientDescriptor);

		scrabble_game_print_available_tiles(scrabbleGameAdd->avTiles, 25);
		
		/* Mechanism for switching players at each turn */
		switch(playerType)
		{
			case FIRST:
				semaphore_unlock(gameSemId, 2, 0);
				break;
			case SECOND:
				semaphore_unlock(gameSemId, 1, 0);
				break;
		}
	};
	
	shared_mem_detach((char*)scrabbleGameAdd);
	free(msg);
	return EXIT;
}

int game_end_check(int movesLeft, int playerType, int gameSemId, int clientDescriptor, int p1Points,int p2Points, char* logName)
{
	if(movesLeft == 0)
	{
		printf(">>>>>>>>>>INSIDE\n");
		packet* msg = malloc(sizeof(packet));
		float response;

		if(msg == NULL)
		{
			ERR("game_end_check() : malloc ");
		}

		msg->msg 		= GAME_ENDED;
		msg->p1Points 	= p1Points;
		msg->p2Points 	= p2Points;
		msg->playerType = playerType;

		/* Log to file */
		log_summary(logName, p1Points, p2Points, "GAME FINISHED");

		/* Unlock appropriate semaphore to let the other */
		/* player (in case of 'out of tales' scenario) get to */
		/* the decision state. */
		switch(playerType)
		{
			case FIRST:
				semaphore_unlock(gameSemId, 2, 0);
				break;
			case SECOND:
				semaphore_unlock(gameSemId, 1, 0);
				break;
		}

		/* Decision state */
		msg->msg = GAME_ENDED;
		tcp_socket_send_packet(clientDescriptor, msg);
		response = tcp_socket_read_packet(clientDescriptor,msg);
		if(msg->msg == PLAY_AGAIN)
		{
			printf("[Client %d] Decided to play again.\n", clientDescriptor);
			printf("[Client %d] Adding to the queue...\n", clientDescriptor);
			free(msg);
			return PLAY_AGAIN;
		}
		free(msg);
		if(INTERRUPTED == response) return INTERRUPTED;
		else if(LOST_CONNECTION == response)    // <--- ?!??! pozmieniac
		{
			printf("[Client %d] Lost connection\n", clientDescriptor);
			return LOST_CONNECTION;

		}
	}
	return PLAYING;
}

void log_entry(char* logName, char c, int x, int y, int p1, int p2)
{
	struct tm tm;
	FILE* log = NULL;
	time_t t = time(NULL);

	tm = *localtime(&t);
	log = fopen(logName,"a");
	if(log == NULL)
	{
		ERR("log_entry() : fopen ");
	}
	if((fprintf(log, "%d-%d-%d %d:%d:%d [IP] letter: %c coordinates(%d,%d) p1_points: %d p2_points: %d\n",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, c, x, y,
			p1, p2)) < 0 )
	{
		ERR("log_entry() : fprintf ");
	}
	fclose(log);
}

void log_summary(char* logName, int p1, int p2, char* message)
{
	struct tm tm;
	FILE* log = NULL;
	time_t t = time(NULL);

	tm = *localtime(&t);
	log = fopen(logName,"a");
	if(log == NULL)
	{
		ERR("log_summary() : fopen");
	}
	if((fprintf(log, "%d-%d-%d %d:%d:%d [IP] p1_points: %d p2_points: %d %s\n",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
			p1, p2, message)) < 0 )
	{
		ERR("log_summary() : fprintf ");
	}
	fclose(log);
}
