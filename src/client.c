#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include "../include/signals_util.h"
#include "../include/scrabble_game.h"
#include "../include/semaphore_util.h"
#include "../include/settings.h"
#include "../include/tcp_socket_util.h"


int make_socket(void);

struct sockaddr_in make_address(char *address, uint16_t port);

/*
 * When SIGINT or SIGTERM is catched, this variable will tell main loop to stop.
 */
volatile sig_atomic_t g_doWork = 1;

void sigint_handler(int sig);

int gather_input(int, char*, int*, int*,int*, char[5], char[5][5]);

int gather_answer(int);

int main(void)
{
    int s, sem;
    //struct sockaddr_in remote;
	packet* rec = malloc(sizeof(packet));
	packet* tmp = malloc(sizeof(packet));
	char c; 
	int x, y, points;
	semaphore_init(&sem, 'E', 1);
	if(sethandler(sigint_handler,SIGINT)) ERR("Setting SIGINT:");
	
	//socket_init_unix(&s,&remote, SOCK_PATH);
	//socket_connect(&s,&remote);
	s = tcp_socket_connect_via_port("localhost", 2000);
	printf("############################\n");
	printf("# WELCOME TO SCRABBLE GAME #\n");
	printf("############################\n");
	

	while(g_doWork)
	{
		if(LOST_CONNECTION == tcp_socket_read_packet(s, rec)) rec->msg = EXIT;
		if(g_doWork == 0) break;
		switch(rec->msg)
		{
			case NO_PLAYER:
				printf("Waiting for another player to connect...\n");
				break;
			case PAIR_MATCH:
				printf("Player found - starting the game...\n");
				break;
			case INFO:
				printf("Player found - starting the game...\n");
				scrabble_game_print_title();
				scrabble_game_print_points(rec->p1Points, rec->p2Points, rec->playerType);
				scrabble_game_print_board(rec->currentBoard);	
				scrabble_game_print_wait_for_move();	
				break;
			case REQUEST_MOVE:
				/* Print points in the right order */
				scrabble_game_print_title();
				scrabble_game_print_points(rec->p1Points, rec->p2Points, rec->playerType);
					
				/* Print current board state */
				scrabble_game_print_board(rec->currentBoard);
				
				/* Print available tiles */
				printf("----------------------------------------\n              ");
				scrabble_game_print_available_tiles(rec->tiles, 5);
				printf("----------------------------------------\n");
				
				points = 0;
				if (INTERRUPTED == gather_input(s, &c, &x, &y, &points, rec->tiles, rec->currentBoard)) break;

				/* Print updated points */
				scrabble_game_print_title();
				if(rec->playerType == FIRST)  rec->p1Points += points;
				if(rec->playerType == SECOND) rec->p2Points += points;
				scrabble_game_print_points(rec->p1Points, rec->p2Points, rec->playerType);
				
				/* Print updated board */
				rec->currentBoard[x][y] = c;
				scrabble_game_print_board(rec->currentBoard);
				
				/* Send data to server. */
				tmp->msg = MOVE_DATA;
				tmp->letter = c;
				tmp->x_coord = x;
				tmp->y_coord = y;
				tmp->p1Points = rec->p1Points;
				tmp->p2Points = rec->p2Points;
				
				tcp_socket_send_packet(s, tmp);
				scrabble_game_print_wait_for_move();
				break;
			case GAME_ENDED:
				scrabble_game_print_game_result(rec->p1Points, rec->p2Points, rec->playerType);
				printf("Would You like to play again?\n");
				if(gather_answer(s) == PLAY_AGAIN)
				{
					tmp->msg = PLAY_AGAIN;
					tcp_socket_send_packet(s,tmp);
				}
				else g_doWork = 0;
				break;
				break;
			case CHOICE_STAGE:
				printf("Your opponent is disconnected. Would You like to play again?\n");
				if(gather_answer(s) == PLAY_AGAIN)
				{
					tmp->msg = PLAY_AGAIN;
					tcp_socket_send_packet(s,tmp);
				}
				else g_doWork = 0;
				break;
			case EXIT:
				printf("Server disconnected.\n");
				g_doWork = 0;
				break;
			default:
				printf("Cannot interpret packet's message\n");
		}
	};

	printf("Disconnected\n");
    if(-1 == close(s)) ERR("Closing client's socket.");
    free(rec);
    free(tmp);
    return 0;
}

int gather_input(int clientFD, char* c, int* x, int* y, int* points, char tiles[5], char board[5][5])
{
	int i, k, j;
	fd_set rset;
	FD_ZERO(&rset);
		
	printf("Supply <letter> <x> <y>\n");
	while(1)
	{
		FD_SET(clientFD, &rset);
		FD_SET(0, &rset);

		if (-1 == select(clientFD+1, &rset, NULL, NULL, NULL))
		{
			return INTERRUPTED;
		}
		if(FD_ISSET(0, &rset)){
			scanf(" %c %d %d", c, x, y);
		}
		if(FD_ISSET(clientFD, &rset)){
			 return INTERRUPTED;
		}
		////////////////////////////////////////////////
		/* Basic border check */
		if(*x < 0 || *y < 0 || *x > 4 || *y > 4)
		{
			printf("Please... x and y must be from <0;4> interval.\n");
			continue;
		}
		
		/* Check if given character is on the list */
		k = 1;
		for(i = 0; i < 5; i++)
			if(tiles[i] == *c && 'x' != *c)
				k = 0;
		if(k == 1)
		{
			printf("Please... letter is not among available ones.\n");
			continue;
		}
		
		/* Check if it is the first move (clean board). */
		k = 1;
		for(i = 0; i < 5; i++)
			for(j = 0; j < 5; j++)	
			{
					if(board[i][j] != 'x')
						k = 0;
			}
		if(k == 1)
		{
			break;
		}
		
		
		/* Check if movement is possible */
		k = 1;
		for(i = -1; i < 2; i++)
			for(j = -1; j < 2; j++)
			{
				int n_x = *x + i;
				int n_y = *y + j;
				
				if(n_x >= 0 && n_x < 5 && n_y >= 0 && n_y < 5)
				{
					if(board[n_x][n_y] != 'x')
						k = 0;
				}
						
			}
		/* Check if tile overlaps other one. */	
		if(board[*x][*y] != 'x')
			k = 1;
		if(k == 1)
		{
			printf("Please... movement is not allowed.\n");
			continue;
		}
		
		
		/* At this point everything (for sure) is ok. */
		break;
	}
	
	*points = scrabble_game_calculate_points(board, *x, *y);

	return DATA_RECEIVED;
}
int gather_answer(int clientFD)
{
	char answer[100];
	fd_set rset;
	FD_ZERO(&rset);
	
	while(1)
	{
		FD_SET(clientFD, &rset);
		FD_SET(0, &rset);

		if (-1 == select(clientFD+1, &rset, NULL, NULL, NULL))
		{
			return INTERRUPTED;
		}
		if(FD_ISSET(0, &rset))
		{
			scanf(" %s", answer);
		}
		if(FD_ISSET(clientFD, &rset))
		{
		 return INTERRUPTED;
		}
		
		if(strcmp(answer, "yes") == 0 ) return PLAY_AGAIN;
		else if (strcmp(answer, "no") == 0) return EXIT;
	}
}

void sigint_handler(int sig)
{
	g_doWork = 0;
}





