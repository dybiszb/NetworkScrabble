#include "../include/tcp_socket_util.h"

int tcp_make_socket(void){
	int sock;

	if((sock = socket(PF_INET,SOCK_STREAM,0)) == -1)
	{
		ERR("tcp_make_socket() : socket ");
	}
	return sock;
}


struct sockaddr_in tcp_get_address(char *address, uint16_t port)
{
	struct sockaddr_in addr;
	struct hostent *hostinfo;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if((hostinfo = gethostbyname(address)) == NULL)
	{
		HERR("tcp_get_address() : gethostbyname ");
	}
	addr.sin_addr = *(struct in_addr*) hostinfo->h_addr;
	return addr;
}

void tcp_socket_init_unix(int *descriptor, struct sockaddr_in* soc)
{
	if ((*descriptor = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
        ERR("tcp_socket_init_unix() : seocket ");
    }

    soc->sin_family = AF_INET;
    soc->sin_port = htons(2000);
	soc->sin_addr.s_addr = htonl(INADDR_ANY);

}

void tcp_socket_bind(int* descriptor, struct sockaddr_in* soc)
{
	int t = 1;

	if (setsockopt(*descriptor, SOL_SOCKET, SO_REUSEADDR,&t, sizeof(t)) == -1)
	{
		ERR("tcp_socket_bind() : setsockopt ");
	}

	if(bind(*descriptor,(const struct sockaddr*) soc,sizeof(*soc)) == -1)
	{
	 	ERR("tcp_socket_bind() : bind ");
	}
}

int tcp_socket_connect_via_port(char *name, uint16_t port)
{
	struct sockaddr_in addr;
	int socketfd;
	socketfd = tcp_make_socket();
	addr=tcp_get_address(name,port);
	if(connect(socketfd,(struct sockaddr*) &addr,sizeof(struct sockaddr_in)) < 0){
		if(errno!=EINTR) ERR("connect");
		else {
			fd_set wfds ;
			int status;
			socklen_t size = sizeof(int);
			FD_ZERO(&wfds);
			FD_SET(socketfd, &wfds);
			if(TEMP_FAILURE_RETRY(select(socketfd+1,NULL,&wfds,NULL,NULL))<0) ERR("select");
			if(getsockopt(socketfd,SOL_SOCKET,SO_ERROR,&status,&size)<0) ERR("getsockopt");
			if(0!=status) ERR("connect");
		}
	}
	return socketfd;
}

void tcp_socket_listen(int* descriptor, int incomingConn)
{
    if (listen(*descriptor, incomingConn) == -1)
	{
        ERR("tcp_socket_listen() : listen ");
    }
}

int tcp_socket_send_packet(int socket, packet* pac)
{
	char serialized[200]; 
	tcp_socket_serialize(*pac, serialized);

	if(send(socket, serialized, sizeof(serialized),0) == -1)
	{
		if(errno == EPIPE || errno == ENOTCONN || errno == EINTR) return NO_CLIENT;
		else ERR("socket_send_packet() : send() ");
	}
	return DATA_SENT;
}

int tcp_wait_for_client(int *clientSocket, int serverSocket, struct sockaddr_in *remote)
{
	socklen_t sockLength = sizeof(remote);
	if ((*clientSocket = accept(serverSocket, (struct sockaddr *)remote, &sockLength)) == -1)
	{
		if(errno == EINTR) return INTERRUPTED;
		else ERR("wait_for_client() : accept()");
	}
	return CONNECTED;
}

int tcp_socket_read_packet(int socket, packet* pac)
{
	int t;
	char serialized[200];

	/* When  a  stream  socket  peer  has  performed  an orderly shutdown,
     * the return value will be 0 (the traditional "end-of-file" return). */
	if ((t = recv(socket, serialized, sizeof(serialized), 0))  == 0)
	{
		/* Client closed connection => activate CHOICE_STATE */
		return LOST_CONNECTION;
	}
	/* -1 if an error occurred */
	else if(t < 0) return INTERRUPTED;

	tcp_socket_deserialize(pac, serialized);

	return DATA_RECEIVED;

}

void tcp_socket_serialize(packet pac, char* str)
{
	sprintf(str, 
	"%d%c%c%c%d%c%d%c%d%c%d%c%d%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 
	
	pac.msg, SEPARATOR, pac.letter, SEPARATOR, pac.x_coord, SEPARATOR, pac.y_coord,
	SEPARATOR, pac.p1Points, SEPARATOR, pac.p2Points, SEPARATOR, pac.playerType,

	SEPARATOR, pac.currentBoard[0][0], SEPARATOR, pac.currentBoard[0][1], SEPARATOR, pac.currentBoard[0][2], SEPARATOR, pac.currentBoard[0][3], SEPARATOR, pac.currentBoard[0][4], 
	SEPARATOR, pac.currentBoard[1][0], SEPARATOR, pac.currentBoard[1][1], SEPARATOR, pac.currentBoard[1][2], SEPARATOR, pac.currentBoard[1][3], SEPARATOR, pac.currentBoard[1][4], 
	SEPARATOR, pac.currentBoard[2][0], SEPARATOR, pac.currentBoard[2][1], SEPARATOR, pac.currentBoard[2][2], SEPARATOR, pac.currentBoard[2][3], SEPARATOR, pac.currentBoard[2][4], 
	SEPARATOR, pac.currentBoard[3][0], SEPARATOR, pac.currentBoard[3][1], SEPARATOR, pac.currentBoard[3][2], SEPARATOR, pac.currentBoard[3][3], SEPARATOR, pac.currentBoard[3][4], 
	SEPARATOR, pac.currentBoard[4][0], SEPARATOR, pac.currentBoard[4][1], SEPARATOR, pac.currentBoard[4][2], SEPARATOR, pac.currentBoard[4][3], SEPARATOR, pac.currentBoard[4][4],
	
	SEPARATOR, pac.tiles[0], SEPARATOR, pac.tiles[1], SEPARATOR, pac.tiles[2], SEPARATOR, pac.tiles[3], SEPARATOR, pac.tiles[4]	

	);
} 

void tcp_socket_deserialize(packet* pac, char* str)
{

	char tmp;
	sscanf(str,
	"%d%c%c%c%d%c%d%c%d%c%d%c%d%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",

	&(pac->msg), &tmp, &(pac->letter), &tmp, &(pac->x_coord), &tmp, &(pac->y_coord),
		&tmp, &(pac->p1Points), &tmp, &(pac->p2Points), &tmp, &(pac->playerType),

	&tmp, &(pac->currentBoard[0][0]), &tmp, &(pac->currentBoard[0][1]), &tmp, &(pac->currentBoard[0][2]), &tmp, &(pac->currentBoard[0][3]), &tmp, &(pac->currentBoard[0][4]),
	&tmp, &(pac->currentBoard[1][0]), &tmp, &(pac->currentBoard[1][1]), &tmp, &(pac->currentBoard[1][2]), &tmp, &(pac->currentBoard[1][3]), &tmp, &(pac->currentBoard[1][4]),
	&tmp, &(pac->currentBoard[2][0]), &tmp, &(pac->currentBoard[2][1]), &tmp, &(pac->currentBoard[2][2]), &tmp, &(pac->currentBoard[2][3]), &tmp, &(pac->currentBoard[2][4]),
	&tmp, &(pac->currentBoard[3][0]), &tmp, &(pac->currentBoard[3][1]), &tmp, &(pac->currentBoard[3][2]), &tmp, &(pac->currentBoard[3][3]), &tmp, &(pac->currentBoard[3][4]),
	&tmp, &(pac->currentBoard[4][0]), &tmp, &(pac->currentBoard[4][1]), &tmp, &(pac->currentBoard[4][2]), &tmp, &(pac->currentBoard[4][3]), &tmp, &(pac->currentBoard[4][4]),

	&tmp, &(pac->tiles[0]), &tmp, &(pac->tiles[1]), &tmp, &(pac->tiles[2]), &tmp, &(pac->tiles[3]), &tmp, &(pac->tiles[4])


	 );

}
