#include "../include/signals_util.h"

int sethandler( void (*f)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	act.sa_flags=SA_SIGINFO;
	sigemptyset( &act.sa_mask );
	if (-1==sigaction(sigNo, &act, NULL))
		return -1;
	return 0;
}
