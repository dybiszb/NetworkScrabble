#include "../include/scrabble_game.h"

void scrabble_game_init(int* id, game* add)
{
	shared_mem_init(id, sizeof(game), 'G');
	add = (game*) shared_mem_attach(*id);
}

void scrabble_game_blank(game* add)
{
	int i, j;
	for(i = 0; i < BOARD_HEIGHT; i++)
	{
		for(j = 0; j < BOARD_WIDTH; j++)
		{
			add->gameBoard[i][j] = 'x';
		}
	}
}

void scrabble_game_print_board(char gameBoard[5][5])
{
	int i, j;
	for(i = 0; i < BOARD_HEIGHT; i++)
	{
		printf("              ");
		for(j = 0; j < BOARD_WIDTH; j++)
		{
			printf("%c ", gameBoard[i][j]);
		}
		printf("\n");
	}
}

void scrabble_game_print_title()
{
	printf("########################################\n");
	printf("#             GAME BOARD               #\n");
	printf("########################################\n");
}

void scrabble_game_attach_tiles(char tiles[25])
{
	char avTiles[25] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 
					  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
					  'Q','R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y'};
	int z;
	for(z = 0; z < 25; z++)
		tiles[z] = avTiles[z];
}

void scrabble_game_print_available_tiles(char tiles[], int n)
{
	int i;
	for(i = 0; i < n; i++) 
	{
		if(tiles[i] != UNAVAILABLE)
			printf("%c ", tiles[i]);
	}
	printf("\n");
}

void scrabble_game_print_wait_for_move()
{
	printf("----------------------------------------\n");
	printf("# Waiting for another player's move... #\n");
	printf("----------------------------------------\n");
}

char scrabble_game_get_random_tile(char tiles[25])
{
	srand(time(NULL));
	char c;
	int r;
	
	while(1)
	{
		r = rand() % 25;
		if(tiles[r] == UNAVAILABLE)
			continue;
		else
		{
			c = tiles[r];
			tiles[r] = UNAVAILABLE;
			return c;
		}
	}
}

void scrabble_game_print_points(int p1, int p2, int type)
{
	printf("----------------------------------------\n");
	switch(type)
	{
		case FIRST:
			printf("  You:      %d\n  Opponent: %d\n", p1, p2);
			break;
		case SECOND:
			printf("  You:      %d\n  Opponent: %d\n", p2, p1);
			break;
	}
	printf("----------------------------------------\n");	
}

void scrabble_game_print_game_result(int p1Points, int p2Points, int type)
{
	if(FIRST == type && p1Points > p2Points)
	{
		printf("########################################\n");
		printf("#                YOU WIN               #\n");
		printf("########################################\n");
		scrabble_game_print_points(p1Points, p2Points, type);
		return;
	}

	if(FIRST == type && p1Points < p2Points)
	{
		printf("########################################\n");
		printf("#               YOU LOOSE               #\n");
		printf("########################################\n");
		scrabble_game_print_points(p1Points, p2Points, type);
		return;
	}
	if(SECOND == type && p2Points > p1Points)
	{
		printf("########################################\n");
		printf("#                YOU WIN               #\n");
		printf("########################################\n");
		scrabble_game_print_points(p1Points, p2Points, type);
		return;
	}

	if(SECOND == type && p2Points < p1Points)
	{
		printf("########################################\n");
		printf("#               YOU LOOSE               #\n");
		printf("########################################\n");
		scrabble_game_print_points(p1Points, p2Points, type);
		return;
	}
}

game scrabble_game_new_game()
{
	game newGame;
	newGame.status = DISCONNECTED;
	scrabble_game_attach_tiles(newGame.avTiles);
	newGame.p1Points = 0;
	newGame.p2Points = 0;
	newGame.movesLeft = 25;
	scrabble_game_blank(&newGame);
	printf("[CLIENT %d] Game has been initialized\n", getpid());
	scrabble_game_print_available_tiles(newGame.avTiles, 25);	
	return newGame;
}

int scrabble_game_calculate_points(char gb[5][5], int x, int y)
{
	int v_p, h_p, d_p, a_p, max;
	
	v_p = 0;
	h_p = 0;
	d_p = 0;
	a_p = 0;
	
	v_p = scrabble_game_calculate_vertical(gb, x ,y);
	max = v_p;
	
	h_p = scrabble_game_calculate_horizontal(gb, x ,y);
	max = (max > h_p) ? max : h_p;
	
	d_p = scrabble_game_calculate_descending(gb, x, y);
	max = (max > d_p) ? max : d_p;	
	
	a_p = scrabble_game_calculate_ascending(gb, x ,y);
	max = (max > a_p) ? max : a_p;	
	
	/* +1 for tile in (x,y). */
	return max + 1;	 
}

int scrabble_game_calculate_vertical(char gb[5][5], int x, int y)
{
	int i, points;
	
	points = 0;
	
	/* Up */
	for(i = (x-1); i>=0; i--)
	{
		if(gb[i][y] == 'x') break;
		points++;
	}
	/* Down */
	for(i = x + 1; i < BOARD_HEIGHT; i++)
	{
		if(gb[i][y] == 'x') break;
		points++;
	}
	
	return points;
}

int scrabble_game_calculate_horizontal(char gb[5][5], int x, int y)
{
	int i, points;
	
	points = 0;
	
	/* Left */
	for(i = (y-1); i>=0; i--)
	{
		if(gb[x][i] == 'x') break;
		points++;
	}
	/* Right */
	for(i = y + 1; i < BOARD_WIDTH; i++)
	{
		if(gb[x][i] == 'x') break;
		points++;
	}
	
	return points;
}

int scrabble_game_calculate_descending(char gb[5][5], int x, int y)
{
	int i, j, points;
	
	points = 0;
	
	/* Up-left */
	for(i = x - 1, j = y - 1; i >= 0 && j>=0; i--, j--)
	{
		if(gb[i][j] == 'x') break;
		points++;
	}
	/* Down-right */
	for(i = x + 1, j = y + 1; i < BOARD_HEIGHT && j < BOARD_WIDTH; i++, j++)
	{
		if(gb[i][j] == 'x') break;
		points++;
	}
	
	return points;
}

int scrabble_game_calculate_ascending(char gb[5][5], int x, int y)
{
	int i, j, points;
	
	points = 0;
	
	/* Down-left */
	for(i = x + 1, j = y - 1; i < BOARD_HEIGHT && j >= 0; i++, j--)
	{
		if(gb[i][j] == 'x') break;
		points++;
	}
	/* Up-right */
	for(i = x - 1, j = y + 1; i >= 0 && j < BOARD_WIDTH; i--, j++)
	{
		if(gb[i][j] == 'x') break;
		points++;
	}
	
	return points;
}


