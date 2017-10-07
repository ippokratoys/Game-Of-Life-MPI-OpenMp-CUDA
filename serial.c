
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/* dimensions of the screen */

#define BOARD_WIDTH	240
#define BOARD_HEIGHT 240

/* set everthing to zero */

void update_out(int**);

/* read a file into the life board */

void rand_init(int **board) {
    int i,j;
	for (j=1; j<(BOARD_HEIGHT+1); j++) {
		for (i=1; i<(BOARD_WIDTH+1); i++) {
            if(rand()%4==0){
                board[j][i] = 0;
            }else{
                board[j][i] = 1;
            }
		}
	}

}

/* main program */

int main (int argc, char *argv[]) {
	int	**board, i, j;

	board=malloc(sizeof(int*)*(BOARD_HEIGHT+2));
	for (i = 0; i < (BOARD_HEIGHT+2); i++) {
		board[i]=malloc(sizeof(int)*(BOARD_WIDTH+2));
	}

	int **newboard=NULL;
	newboard=malloc(sizeof(int*)*(BOARD_WIDTH+2));

	for ( i = 0; i < (BOARD_HEIGHT+2); i++) {
		newboard[i]=malloc(sizeof(int)*(BOARD_WIDTH+2));
	}


	rand_init (board);

	/* play game of life 100 times */
    clock_t start_t, end_t, total_t;
    double cpu_time_used;
    start_t = clock();
	int a;
	int cur_loop;
	for (cur_loop=0; cur_loop<10000; cur_loop++) {
		update_out(board);
		for (i=1; i<(BOARD_WIDTH-1); i++){
			for (j=1; j<(BOARD_HEIGHT-1); j++) {
				a=0;
				a+=board[i-1][j-1];
				a+=board[i-1][j];
				a+=board[i-1][j+1];

				a+=board[i][j-1];
				a+=board[i][j+1];

				a+=board[i+1][j-1];
				a+=board[i+1][j];
				a+=board[i+1][j+1];

				if (a == 2) newboard[i][j] = board[i][j];
				if (a == 3) newboard[i][j] = 1;
				if (a < 2) newboard[i][j] = 0;
				if (a > 3) newboard[i][j] = 0;
			}
		}

		/* copy the new board back into the old board */
		int** temp;
		temp=board;
		board=newboard;
		newboard=temp;
	}
    end_t = clock();
    total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
    cpu_time_used=(double)(end_t - start_t) / CLOCKS_PER_SEC;
    printf("(%3dx%3d)Total time taken by CPU: %f\n",BOARD_HEIGHT,BOARD_WIDTH,cpu_time_used  );
}

void update_out(int** board) {
	int i,j;
	//update the first and last line
	for (j = 1; j <(BOARD_WIDTH+1); j++) {
		board[0][j]=board[BOARD_WIDTH-2][j];
		board[BOARD_WIDTH-1][j]=board[1][j];
	}
	for (i = 1; i <(BOARD_WIDTH+1); i++) {
		board[i][0]=board[i][BOARD_WIDTH-2];
		board[i][BOARD_WIDTH-1]=board[i][1];
	}
	board[0][0]=board[BOARD_WIDTH-2][BOARD_WIDTH-2];
	board[0][BOARD_WIDTH-1]=board[BOARD_WIDTH-2][1];
	board[BOARD_WIDTH-1][0]=board[1][BOARD_WIDTH-2];
	board[BOARD_WIDTH-1][BOARD_WIDTH-1]=board[1][1];

}
