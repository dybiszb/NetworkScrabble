//==============================================================================
// All settings, messages and constants macros are defined here.
// =============================================================================
// author: dybisz
//------------------------------------------------------------------------------

#ifndef SETTINGS_H
#define SETTINGS_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h> 
#include <errno.h>

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))

#define HERR(source) (fprintf(stderr,"%s(%d) at %s:%d\n",source,h_errno,\
                      __FILE__,__LINE__), exit(EXIT_FAILURE))

#define SEPARATOR			';'
#define UNAVAILABLE			'!'
#define MAX_CHILDREN		 100
#define INCOMMING_CONN 		 5
#define BOARD_WIDTH			 5
#define BOARD_HEIGHT		 5
#define NO_PLAYER			-1
#define PAIR_MATCH			-2
#define NONE				-3
#define FIRST				-4
#define SECOND				-5
#define DISCONNECTED		-6
#define CONNECTED			-7
#define REQUEST_MOVE		-8
#define MOVE_DATA			-9
#define INFO				-10
#define EXIT				-11
#define WAITING_PLAYER		-12
#define INTERRUPTED			-13
#define ACUIRED				-14
#define NOT_AVAILABLE		-15
#define DATA_RECEIVED		-16
#define INPUT_READ			-17
#define LOST_CONNECTION		-18
#define CHOICE_STAGE		-19
#define UNLOCKED			-20
#define PLAY_AGAIN			-21
#define NO_CLIENT			-22
#define DATA_SENT			-23
#define GAME_ENDED			-24
#define PLAYING				-25

#endif
