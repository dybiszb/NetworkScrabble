# Network Scrabble
## Project for 'UNIX' course at Warsaw University of Technology

## Technology:
- C
- UNIX System V (semaphores, shared memory)
- Unix Processes Management (multiprocessing, signals)
- TCP sockets

## Screenshots

Server Sample           |  Client Sample
:-------------------------:|:-------------------------:
    ![alt tag](https://raw.githubusercontent
    .com/dybiszb/NetworkScrabble/master/img/client_scr.png  |  ![alt tag](https://raw.githubusercontent
                                                                                .com/dybiszb/NetworkScrabble/master/img/server_scr.png)

## Task Description
Very simplified "scrabble" game. Server joins players in pairs to play one game ( matched automatically as new connections are accepted). After the game is finished clients are informed about result and are given the choice to play once more or exit. At the same time server runs multiple games e.g. if 9 peers connects it starts 4 games and one peer must wait for another connection.

If one of peers disconnects before game finish, the other peer must be given the same choice as if the game ended. The game is unresolved.

All game moves and results are logged in special file. Single log entry must consists of date, peers IP's, set of moves and the game result (points/unresolved), all in one block in the file.

Program must be implemented on processes (not threads) with use of sysV shared memory and semaphores. Each client connection must be handled by it's own process.

Game Rules

- There are only 25 tiles: A B C D E F G H I J K L M N O P Q R S T U V W X Y
- Board is 5 x 5 grid, no bonus cell's
- Each player holds maximum 5 tiles, tiles are selected randomly
- Moves are made in turns in the same order players were joining the game
- In one move player can place only one tile, first player in first turn can place first tile at any position on board, next tiles must always touch other tile
- Points gained are equal to the length of the longest ascending or descending, vertical or horizontal sequence of tiles crossing the newly placed tile.
- After each move, the player gets new random tile.
- Game ends when there are no more tiles left, gained points show the winner

## Compiling<a name="compile"></a>
Note: For obvious reasons only UNIX compilation is allowed.

In repository main folder call:

```
make all
```

Two executables should be created: server and client.

## Run<a name="run"></a>

First of all, run a server calling:
```
./server
```
in repository folder. At this point new clients can connect. Each of them
must be started in new console window via:
```
./client
```
If you want to break connection press CTRL-C. All controls are described
during the client application runtime.