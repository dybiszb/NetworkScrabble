//==============================================================================
// A set of functions that constitutes an interface for tcp socket
// communication. One can e.g. connect, disconnect and send specialized
// packed over a network.
// =============================================================================
// author: dybisz
//------------------------------------------------------------------------------

#ifndef TCP_SOCKET_UTIL_H
#define TCP_SOCKET_UTIL_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "settings.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>

/*
 * Structure encapsulates data sent over socket.
 */
typedef struct
{
	int msg;
	char letter;
	int x_coord;
	int y_coord;
	int p1Points;
	int p2Points;
	int playerType;
	char currentBoard[BOARD_HEIGHT][BOARD_WIDTH];
	char tiles[5];
} packet;

/**
 * Creates TCP socket;
 */
int tcp_make_socket(void);

/**
 * Using list of addresses and port - get appropriate address structure.
 */
struct sockaddr_in tcp_get_address(char *address, uint16_t port);

/*
 * Creates basic socket of AF_UNIX family. 
 * Socket is indetified with its descriptor and special structure.
 */
void tcp_socket_init_unix(int *descriptor, struct sockaddr_in* soc);

/*
 * Binds given descriptor and socket structure.
 */
void tcp_socket_bind(int*, struct sockaddr_in*);

/**
 * Connect to the socket using provided port.
 */
int tcp_socket_connect_via_port(char *name, uint16_t port);

/*
 * Forces socket to start listening for incoming connections.
 * Number of queued connections (before accept()) must be provided.
 */
void tcp_socket_listen(int*, int);

/*
 * Sends provided packet over a socket.
 */
int tcp_socket_send_packet(int, packet*);

/*
 * Reads packet from a socket.
 */
int tcp_socket_read_packet(int, packet*);

/*
 * Serialize packet;
 */
void tcp_socket_serialize(packet pac, char* str);

/*
 * Deserialize packet;
 */
void tcp_socket_deserialize(packet* pac, char* str);

/*
 * Get new client's socket id.
 */
int tcp_wait_for_client(int *, int, struct sockaddr_in *);

#endif
