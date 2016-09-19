//==============================================================================
// Set of function related to managing scrabble game via sysV's shared memory.
// =============================================================================
// author: dybisz
//------------------------------------------------------------------------------

#ifndef SCRABBLE_GAME_H
#define SCRABBLE_GAME_H

#include "settings.h"
#include "shared_mem_util.h"

/**
 *  Structure of a game that will be hold in shared memory.
 *  Brief entries decription:
 *  [gameBoard] Array with current state of a game board.
 *  [status]    Current game status (CONNECTED, DISCONNECTED, NO_PLAYER)
 *  [avTiles]   Tiles that are still available. Those which are not will
 *              be marked as UNAVAILABLE (see settings.h).
 *  [p1Points]  Points of a first player.
 *  [p2Points]  Points of a second player.
 *  [movesLeft] How many moves left till the game ends.
 */
typedef struct
{
	char gameBoard[BOARD_HEIGHT][BOARD_WIDTH];
	int  status;
	char avTiles[25];
	int  p1Points;
	int  p2Points;
	int  movesLeft;
} game;

/**
 *  All entries of supplied game board are set to 'x'.
 *
 *  @param game* Pointer to game struct.
 */
void scrabble_game_blank(game*);

/**
 * Prints provided array in a console.
 *
 * @param gameBoard 5x5 array of chars.
 */
void scrabble_game_print_board(char gameBoard[5][5]);

/**
 *  Simple 'GAME BOARD' title is printed in a console.
 */
void scrabble_game_print_title();

/**
 *  Fills provided array with following entries:
 *  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H','I', 'J', 'K', 'L', 
 *  'M', 'N', 'O', 'P', 'Q','R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y'
 *
 *  NOTE: I know that the implementation should be realized via memcopy procedure.
 *        It is not.
 *
 *  @param tiles Chars array of length 25.
 */
void scrabble_game_attach_tiles(char tiles[25]);

/**
 * Prints out supplied list of available tiles.
 *
 * @param tiles   List of tiles that will be printed.
 * @param length  Length of list the list.
 */
void scrabble_game_print_available_tiles(char tiles[], int length);

/**
 * Prints out information about waiting for other player's move.
 */
void scrabble_game_print_wait_for_move();

/**
 * Randomly picks up a letter from provided array. array may contain entries
 * marked as UNAVAILABLE, which cannot be chosen.
 *
 * NOTE: The author realizes that following procedure is implemented correctly
 *       but in a very unefficient way.
 *
 * @param tiles Array of chars of length 25.
 *
 * @return      Randomly picked character.
 */
char scrabble_game_get_random_tile(char tiles[25]);

/**
 * Prints points of selected player. Both player 1 and 2 points are assumed to be
 * passed due to keeping information in a shared memory.
 *
 * @param p1   Player 1 points.
 * @param p2   Player 2 points.
 * @param type Type of player (FIRST, SECOND).
 */
void scrabble_game_print_points(int p1, int p2, int type);

/**
 * Calculates points for a given field on a board.
 *
 * @param game_board 5x5 chars array.
 * @param x          X coordinate of a board entry.
 * @param y          Y coordinate of a board entry.
 *
 * @return           Number of points.
 */
int scrabble_game_calculate_points(char game_board[5][5], int x, int y);

/**
 * Calculates vertical points for a given field on a board.
 *
 * @param game_board 5x5 chars array.
 * @param x          X coordinate of a board entry.
 * @param y          Y coordinate of a board entry.
 *
 * @return           Number of points.
 */
int scrabble_game_calculate_vertical(char game_board[5][5], int x, int y);

/**
 * Calculates horizontal points for a given field on a board.
 *
 * @param game_board 5x5 chars array.
 * @param x          X coordinate of a board entry.
 * @param y          Y coordinate of a board entry.
 *
 * @return           Number of points.
 */
int scrabble_game_calculate_horizontal(char game_board[5][5], int x, int y);

/**
 * Calculates descending points for a given field on a board.
 *
 * @param game_board 5x5 chars array.
 * @param x          X coordinate of a board entry.
 * @param y          Y coordinate of a board entry.
 *
 * @return           Number of points.
 */
int scrabble_game_calculate_descending(char game_board[5][5], int x, int y);

/**
 * Calculates ascending points for a given field on a board.
 *
 * @param game_board 5x5 chars array.
 * @param x          X coordinate of a board entry.
 * @param y          Y coordinate of a board entry.
 *
 * @return           Number of points.
 */
int scrabble_game_calculate_ascending(char game_board[5][5], int x, int y);

/**
 * Prints out appropriate message (based on supplied results and player type).
 *
 * @param p1Points Number of points of first player.
 * @param p2Points Number of points of second player.
 * @param type     Who is aking - player 1 (FIRST) or player 2 (SECOND).
 */
void scrabble_game_print_game_result(int p1Points, int p2Points, int type);

/**
 * Generates new game struct with default values and prints out a message about
 * succesfull initialization along with player's pid.
 *
 * @return 'game' struct object with default values.
 */
game scrabble_game_new_game();

#endif
