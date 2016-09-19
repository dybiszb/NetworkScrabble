//==============================================================================
// A set of functions that manages a server.
// =============================================================================
// author: dybisz
//------------------------------------------------------------------------------

#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "settings.h"
#include "signals_util.h"
#include "semaphore_util.h"
#include "shared_mem_util.h"
#include "scrabble_game.h"
#include "tcp_socket_util.h"

/* Structure for the queue members. */
typedef struct
{
	int status;
	int gameSharedMemory;
	int gameSemaphore;
	char logName[100];
} PlayerInfo;

/*
 * Each client after accepting, gets his own process and permorms this function.
 */
void handle_client(int, int, PlayerInfo*);

/*
 * When ...
 */
void sigint_handler(int sig);

/*
 * When ...
 */
void sigchld_handler(int);
void init_game(int*);
void send_msg(int, packet*, int);
void write_to_queue(PlayerInfo*, int, int, int,  char*);
void read_from_queue(PlayerInfo*, int*, int*, char*);
int check_for_another_client(PlayerInfo*,int, int, int, int);
void write_to_packet(packet*, int, char, int, int, int, int, int,char[5][5]);
int main_game_loop(int, int, int, char*, int);
void local_cleanup(int, packet*, game*, int*, int*);
void close_socket(int);
int game_end_check(int,int,int,int, int, int, char*);
void log_entry(char* logName, char c, int x, int y, int p1, int p2);
void log_summary(char* logName, int p1, int p2, char* message);
#endif
