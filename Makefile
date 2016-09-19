CC = gcc
COMPILE_FLAGS = -Wall 
OBJ =  semaphore_util.o shared_mem_util.o signals_util.o scrabble_game.o tcp_socket_util.o
SOURCES_DIR=src
all: server client clean

.PHONY: all clean

server: server.o ${OBJ}
	${CC} -o server server.o ${OBJ}

client: client.o ${OBJ}
	${CC} -o client client.o ${OBJ}

server.o: ${SOURCES_DIR}/server.c
	${CC} -o server.o -c ${SOURCES_DIR}/server.c ${COMPILE_FLAGS}

client.o: ${SOURCES_DIR}/client.c
	${CC} -o client.o -c ${SOURCES_DIR}/client.c ${COMPILE_FLAGS}

semaphore_util.o: ${SOURCES_DIR}/semaphore_util.c
	${CC} -o semaphore_util.o -c ${SOURCES_DIR}/semaphore_util.c ${COMPILE_FLAGS}

shared_mem_util.o: ${SOURCES_DIR}/shared_mem_util.c
	${CC} -o shared_mem_util.o -c ${SOURCES_DIR}/shared_mem_util.c ${COMPILE_FLAGS}

signals_util.o: ${SOURCES_DIR}/signals_util.c
	${CC} -o signals_util.o -c ${SOURCES_DIR}/signals_util.c ${COMPILE_FLAGS}

scrabble_game.o: ${SOURCES_DIR}/scrabble_game.c 
	${CC} -o scrabble_game.o -c ${SOURCES_DIR}/scrabble_game.c ${COMPILE_FLAGS}

tcp_socket_util.o: ${SOURCES_DIR}/tcp_socket_util.c
	${CC} -o tcp_socket_util.o -c ${SOURCES_DIR}/tcp_socket_util.c ${COMPILE_FLAGS}

clean:
	rm server.o client.o ${OBJ}
