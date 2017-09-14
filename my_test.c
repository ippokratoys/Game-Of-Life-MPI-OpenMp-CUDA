#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>

#define ALIVE 'X'
#define DEAD '.'
#define FILENAME "input"

#define SEND_RECIEVE 1
#define FILE_OUT 1

#define UP_LEFT 0
#define UP 1
#define UP_RIGHT 2
#define RIGHT 3
#define DOWN_RIGHT 4
#define DOWN 5
#define DOWN_LEFT 6
#define LEFT 7

#define LOOPS 2
int main(void){
	int i=0,j,temp,temp1;
	int num_proc;
	int my_rank;

	char **board;//the sub board
	int size;//the size of the global board(NxN)
	int sub_size;//the size of the sub board

	int bufc;

	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD,&num_proc);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank );

    FILE *input=fopen(FILENAME,"r");//open the file for the init state
	fscanf(input,"%d",&size);//read the size of the board

	sub_size=sqrt(size*size/num_proc);//the size of the subarray nXn

	board=malloc(sizeof(char*)*sub_size);//allocate mem for the sub_board
	int first_line=0;//from which line should start reading
	int cycle_size=0;//every x process change line
	int first_col=0;//from which column should start
	int file_line_offset=0;//how much i must move in the file for one line
	int correct_offset=0;//goes to the next to read, no to the same place next line

	cycle_size=size/sub_size;// CAREFULLL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	first_line=(my_rank/cycle_size)*(sub_size);
	first_col=(my_rank%cycle_size)*(sub_size);
	file_line_offset=size+1;//because of the \n
	correct_offset=file_line_offset-sub_size;

	if(my_rank==0){//diagnostic messages for debugg
		printf("------------\n");
		printf("size:%d\n",size);
		printf("sub_size:%d\n",sub_size);
		printf("cycle_size:%d\n",cycle_size);
		printf("-------------\n");
	}
	//limits of each subarray
	printf("process %d sub_array limits line:%d col:%d\n",my_rank,first_line,first_col);
////////////////do the reading////////////////
	int seek;
	seek=1+file_line_offset*first_line;//move to the correct line
	seek+=first_col;//move to the correct column
	fseek(input,seek,SEEK_CUR);
	for(i=first_line;i<first_line+sub_size;i++){
		board[i-first_line]=malloc(sizeof(char)*sub_size);//0,1,2..
		//offset move in the file correct line and row
		for(j=0;j<sub_size;j++){
			bufc=fgetc(input);
//			if(j==0)printf("rank:%d char:|%c|\n",my_rank,bufc);
//			if(bufc!=ALIVE && bufc!=DEAD )printf("error reading|%c|\n",bufc);
			if(bufc!=DEAD && bufc!=ALIVE){
				printf("LOOOL rank:%d-i=%d,j=%d\n",my_rank,i,j);
			}
			board[i-first_line][j]=bufc;
		}//be carefull of the \n
		fseek(input,correct_offset,SEEK_CUR);//go to the next line
	}
	printf("!%d end with reading\n",my_rank);
	fclose(input);//close the file of input
////////////////end of reading////////////////
	int neighbors=0;//number of allive neighbors
	char **board_new,**tmp_p;//pointer for the new buffer board and a temp
	char *temp_col_first=malloc(sizeof(char)*sub_size);//buffer for the first row
	char *temp_col_last=malloc(sizeof(char)*sub_size);//buffer for the last row

	board_new=malloc(sizeof(char*)*sub_size);
	for(i=0;i<sub_size;i++){
		board_new[i]=malloc(sizeof(char)*sub_size);
	}
	int loops=LOOPS;
/////////////////////////// INITIALIZE THE COMMUNICATIONS///////////////////////
	cycle_size--;
//the buffers for the recieve
	char rec_u_l,rec_u_r,rec_d_l,rec_d_r;

	int up=my_rank-(cycle_size+1);
	char *up_row=malloc(sizeof(char)*sub_size);

	int down=my_rank+(cycle_size+1);
	char *down_row=malloc(sizeof(char)*sub_size);

	int left=my_rank-1;
	char *left_col=malloc(sizeof(char)*sub_size);

	int right=my_rank+1;
	char *right_col=malloc(sizeof(char)*sub_size);

	int up_left=my_rank-(cycle_size+1)-1;char up_left_char;
	int up_right=my_rank-(cycle_size+1)+1;char up_right_char;
	int down_left=my_rank+(cycle_size+1)-1;char down_left_char;
	int down_right=my_rank+(cycle_size+1)+1;char down_right_char;

	if(my_rank<=cycle_size){//if we are at the first line
		up=(num_proc)-(cycle_size+1)+my_rank;
		up_right=up+1;
		up_left=up-1;
		if(my_rank==0){//zero process
			left=cycle_size;
			up_left=num_proc-1;
			down_left=2*(cycle_size+1)-1;

		}else if( my_rank==cycle_size ){//last of the first row
			right=0;
			up_right=(num_proc-1)-cycle_size;
			down_right=my_rank+1;
		}
	}else if(my_rank>(num_proc-1)-(cycle_size+1) ){//if we are at the last line
		down=my_rank%(cycle_size+1);
		down_left=down-1;
		down_right=down+1;
		if(my_rank%(cycle_size+1)==0){//first of last row
			left=(num_proc-1);
			up_left=my_rank-1;
			down_left=cycle_size;
		}else if( (my_rank+1)%(cycle_size+1) == 0){//last of all
			right=my_rank-cycle_size;//send the first col
			up_right=my_rank-2*(cycle_size+1)+1;//the up right
			down_right=0;//the down rights
		}
	}else{//general case for the line
		if(my_rank%(cycle_size+1)==0){//first col
			left=my_rank+cycle_size;//send the first col
			up_left=my_rank-1;//the up left
			down_left=my_rank+2*(cycle_size+1)-1;//the down left

		}else if( (my_rank+1)%(cycle_size+1) == 0){//last col
			right=my_rank-cycle_size;//send the first col
			up_right=my_rank-2*(cycle_size+1)+1;//the up right
			down_right=my_rank+1;//the down rights
		}
	}

////////////////////////////the MAIN loop///////////////////////////////////////
	for(i=0;i<sub_size;i++){
		temp_col_first[i]=board[i][0];
		temp_col_last[i]=board[i][sub_size-1];
	}
	while(loops){
//i send and i recieve of the outer part
#if SEND_RECIEVE
//cycle size is 0-base
//

		//rows
		MPI_Send(board[0],sub_size,MPI_BYTE,up,DOWN,MPI_COMM_WORLD);//send the first line
		MPI_Send(board[sub_size-1],sub_size,MPI_BYTE,down,UP,MPI_COMM_WORLD);//send the last line

		//columns
		MPI_Send(temp_col_first,sub_size,MPI_BYTE,left,RIGHT,MPI_COMM_WORLD);//send the first col
		MPI_Send(temp_col_last,sub_size,MPI_BYTE,right,LEFT,MPI_COMM_WORLD);//send the last col

		//corners
		MPI_Send(&board[0][0],1,MPI_BYTE,up_left,DOWN_RIGHT,MPI_COMM_WORLD);//send the up left
		MPI_Send(&board[0][sub_size-1],1,MPI_BYTE,up_right,DOWN_LEFT,MPI_COMM_WORLD);//send the up right
		MPI_Send(&board[sub_size-1][0],1,MPI_BYTE,down_left,UP_RIGHT,MPI_COMM_WORLD);//send the down left
		MPI_Send(&board[sub_size-1][sub_size-1],1,MPI_BYTE,down_right,UP_LEFT,MPI_COMM_WORLD);//send the down right

#endif
//update the inner part
		printf("(%d) updates the inner part start\n", my_rank);
		for(i=1;i<sub_size-1;i++){//for the inner part of the table
			for(j=1;j<sub_size-1;j++){
				neighbors=0;//count the allive
				if( board[i-1][j-1]==ALIVE )neighbors++;
				if( board[i-1][j]  ==ALIVE )neighbors++;
				if( board[i-1][j+1]==ALIVE )neighbors++;
				if( board[i][j-1]  ==ALIVE )neighbors++;
				if( board[i][j+1]  ==ALIVE )neighbors++;
				if( board[i+1][j-1]==ALIVE )neighbors++;
				if( board[i+1][j]  ==ALIVE )neighbors++;
				if( board[i+1][j+1]==ALIVE )neighbors++;

//				if(neighbors==2)printf("!!!!!!!!!%d\n",neighbors);
//find if should be dead or alive in the next generation
				if( (neighbors==2 && board[i][j]==ALIVE) || neighbors==3 ){
//					printf("aalliiive\n");
					board_new[i][j]=ALIVE;
				}else{
//					printf("IT'S * DEAD\n");
					board_new[i][j]=DEAD;
				}
			}

		}//end of the inner part
//do the recieve for the outer part

#if SEND_RECIEVE
		//rows
		MPI_Recv(up_row,sub_size,MPI_BYTE,up,UP,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		MPI_Recv(down_row,sub_size,MPI_BYTE,down,DOWN,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

		//columns
		MPI_Recv(left_col,sub_size,MPI_BYTE,left,LEFT,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		MPI_Recv(right_col,sub_size,MPI_BYTE,right,RIGHT,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

		//corners
		MPI_Recv(&rec_u_l,1,MPI_BYTE,up_left,UP_LEFT,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		MPI_Recv(&rec_u_r,1,MPI_BYTE,up_right,UP_RIGHT,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		MPI_Recv(&rec_d_l,1,MPI_BYTE,down_left,DOWN_LEFT,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		MPI_Recv(&rec_d_r,1,MPI_BYTE,down_right,DOWN_RIGHT,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
#endif

//update the outer part
		printf("(%d) updates the outer part start\n", my_rank);
	//the first line (without the corneres)
		i=0;
		if(my_rank==0){
			printf("%d\n",(num_proc-1)-(cycle_size+1));
		}
		for(j=1;j<sub_size-1;j++){
			neighbors=0;//count the allive
			if( up_row[j-1]==ALIVE )neighbors++;
			if( up_row[j]  ==ALIVE )neighbors++;
			if( up_row[j+1]==ALIVE )neighbors++;
			if( board[i][j-1]  ==ALIVE )neighbors++;
			if( board[i][j+1]  ==ALIVE )neighbors++;
			if( board[i+1][j-1]==ALIVE )neighbors++;
			if( board[i+1][j]  ==ALIVE )neighbors++;
			if( board[i+1][j+1]==ALIVE )neighbors++;
	//find if should be dead or alive in the next generation
			if( (neighbors==2 && board[i][j]==ALIVE) || neighbors==3 ){
				board_new[i][j]=ALIVE;
			}else{
				board_new[i][j]=DEAD;
			}
		}
		printf("(%d) \tupdates the last line\n", my_rank);
		//for the last line
		i=sub_size-1;
		for(j=1;j<sub_size-1;j++){
			neighbors=0;//count the allive
			if( board[i-1][j-1]==ALIVE )neighbors++;
			if( board[i-1][j]  ==ALIVE )neighbors++;
			if( board[i-1][j+1]==ALIVE )neighbors++;
			if( board[i][j-1]  ==ALIVE )neighbors++;
			if( board[i][j+1]  ==ALIVE )neighbors++;
			if( down_row[j-1]==ALIVE )neighbors++;
			if( down_row[j]  ==ALIVE )neighbors++;
			if( down_row[j+1]==ALIVE )neighbors++;

		//find if should be dead or alive in the next generation
			if( (neighbors==2 && board[i][j]==ALIVE) || neighbors==3 ){
				board_new[i][j]=ALIVE;
			}else{
				board_new[i][j]=DEAD;
			}
		}
		//for the first collum
		printf("(%d) \tupdates the first col\n", my_rank);
		j=0;
		for(i=1;i<sub_size-1;i++){
			neighbors=0;//count the allive
			if( left_col[i-1]==ALIVE )neighbors++;
			if( board[i-1][j]  ==ALIVE )neighbors++;
			if( board[i-1][j+1]==ALIVE )neighbors++;
			if( left_col[i]  ==ALIVE )neighbors++;
			if( board[i][j+1]  ==ALIVE )neighbors++;
			if( left_col[i+1]==ALIVE )neighbors++;
			if( board[i+1][j]  ==ALIVE )neighbors++;
			if( board[i+1][j+1]==ALIVE )neighbors++;

//find if should be dead or alive in the next generation
			if( (neighbors==2 && board[i][j]==ALIVE) || neighbors==3 ){
				board_new[i][j]=ALIVE;
			}else{
				board_new[i][j]=DEAD;
			}
			temp_col_first[i]=board_new[i][j];
		}
		//for the last collum
		j=sub_size-1;
		printf("(%d) \tupdates the last col\n", my_rank);
		for(i=1;i<sub_size-1;i++){
			neighbors=0;//count the allive

			if( board[i-1][j-1]==ALIVE )neighbors++;
			if( board[i-1][j]  ==ALIVE )neighbors++;
			if( right_col[i-1]==ALIVE )neighbors++;
			if( board[i][j-1]  ==ALIVE )neighbors++;
			if( right_col[i]  ==ALIVE )neighbors++;
			if( board[i+1][j-1]==ALIVE )neighbors++;
			if( board[i+1][j]  ==ALIVE )neighbors++;
			if( right_col[i+1]==ALIVE )neighbors++;

//find if should be dead or alive in the next generation
			if( (neighbors==2 && board[i][j]==ALIVE) || neighbors==3 ){
				board_new[i][j]=ALIVE;
			}else{
				board_new[i][j]=DEAD;
			}
			temp_col_last[i]=board_new[i][j];
		}
		//up_left`
		printf("(%d) \tupdates the up_left col\n", my_rank);
		i=0;j=0;
		neighbors=0;//count the allive
		if( rec_u_l==ALIVE )neighbors++;
		if( up_row[j]  ==ALIVE )neighbors++;
		if( up_row[j+1]==ALIVE )neighbors++;
		if( left_col[i]  ==ALIVE )neighbors++;
		if( board[i][j+1]  ==ALIVE )neighbors++;
		if( left_col[i+1]==ALIVE )neighbors++;
		if( board[i+1][j]  ==ALIVE )neighbors++;
		if( board[i+1][j+1]==ALIVE )neighbors++;
		if( (neighbors==2 && board[i][j]==ALIVE) || neighbors==3 ){
			board_new[i][j]=ALIVE;
		}else{
			board_new[i][j]=DEAD;
		}
		temp_col_first[i]=board_new[i][j];
		//up_rirght
		printf("(%d) \tupdates the up_right col\n", my_rank);
		i=0;j=sub_size-1;
		neighbors=0;//count the allive
		if( up_row[j-1]==ALIVE )neighbors++;
		if( up_row[j]  ==ALIVE )neighbors++;
		if( rec_u_r==ALIVE )neighbors++;
		if( board[i][j-1]  ==ALIVE )neighbors++;
		if( left_col[i]  ==ALIVE )neighbors++;
		if( board[i+1][j-1]==ALIVE )neighbors++;
		if( board[i+1][j]  ==ALIVE )neighbors++;
		if( left_col[i+1]==ALIVE )neighbors++;
		if( (neighbors==2 && board[i][j]==ALIVE) || neighbors==3 ){
			board_new[i][j]=ALIVE;
		}else{
			board_new[i][j]=DEAD;
		}
		temp_col_last[i]=board_new[i][j];

		//down left
		printf("(%d) \tupdates the down_left col\n", my_rank);
		i=sub_size-1;j=0;
		neighbors=0;//count the allive
		if( left_col[i-1]==ALIVE )neighbors++;
		if( board[i-1][j]  ==ALIVE )neighbors++;
		if( board[i-1][j+1]==ALIVE )neighbors++;
		if( left_col[i]  ==ALIVE )neighbors++;
		if( board[i][j+1]  ==ALIVE )neighbors++;
		if( rec_d_l==ALIVE )neighbors++;
		if( down_row[j]  ==ALIVE )neighbors++;
		if( down_row[j+1]==ALIVE )neighbors++;
		if( (neighbors==2 && board[i][j]==ALIVE) || neighbors==3 ){
			board_new[i][j]=ALIVE;
		}else{
			board_new[i][j]=DEAD;
		}
		temp_col_first[i]=board_new[i][j];

		//down right
		printf("(%d) \tupdates the down_right col\n", my_rank);
		i=sub_size-1;j=sub_size-1;
		neighbors=0;//count the allive
		if( board[i-1][j-1]==ALIVE )neighbors++;
		if( board[i-1][j]  ==ALIVE )neighbors++;
		if( right_col[i-1] ==ALIVE )neighbors++;
		if( board[i][j-1]  ==ALIVE )neighbors++;
		if( right_col[i]   ==ALIVE )neighbors++;
		if( down_row[j-1]==ALIVE )neighbors++;
		if( down_row[j]  ==ALIVE )neighbors++;
		if( rec_d_r==ALIVE )neighbors++;
		if( (neighbors==2 && board[i][j]==ALIVE) || neighbors==3 ){
			board_new[i][j]=ALIVE;
		}else{
			board_new[i][j]=DEAD;
		}
		temp_col_last[i]=board_new[i][j];

//move the buffer to the new and use the old as buffer
		tmp_p=board;
		board=board_new;
		board_new=tmp_p;
		loops--;
	}

//write to file for output
	//loop for free each line
	free(board_new);
	FILE* out;
	char out_file[10];
	sprintf(out_file,"out%d",my_rank);
	out=fopen(out_file,"w");
	fprintf(out,"i send to:\n");
	fprintf(out," up:%d\n up right:%d\n right:%d\n down right:%d\n down:%d\n down_left:%d\n left:%d\n up_left:%d\n",up,up_right,right,down_right,down,down_left,left,up_left);
	fprintf(out, "---------------------------------------------------------\n");
	for(i=0;i<sub_size;i++){
		for(j=0;j<sub_size;j++){
			fprintf(out,"%c",board[i][j]);
		}
		fprintf(out,"\n");
	}
	fprintf(out, "---------------------------------------------------------\n");
	fclose(out);//close the file with the out of the cur proc
#if FILE_OUT
	sprintf(out_file,"%s_out_%d",FILENAME,LOOPS);

	char start=1;
	char* empty_line;
	empty_line=malloc(sizeof(char)*sub_size);
	printf("(%d) Will start writting in the file\n", my_rank);
	if(my_rank==0){
		out=fopen(out_file,"a");
		fprintf(out,"%d", size);//write the size at the first line
		fclose(out);
		for(i=0;i<sub_size;i++){
			out=fopen(out_file,"a");
			fprintf(out,"\n", size);//write the size at the first line
			for(j=0;j<sub_size;j++){
				fprintf(out,"%c",board[i][j]);
			}
			fclose(out);
			MPI_Send(&start,1,MPI_BYTE,my_rank+1,RIGHT,MPI_COMM_WORLD);//leave next to write
			MPI_Recv(&start,1,MPI_BYTE,my_rank+cycle_size,MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);//wait from last of the line
		}
		MPI_Send(&start,1,MPI_BYTE,my_rank+cycle_size+1,my_rank,MPI_COMM_WORLD);//send start single to the next line
	}else{
		if(my_rank==num_proc-(cycle_size+1)){//if it's the first of the last line
			MPI_Recv(&start,1,MPI_BYTE,my_rank-(cycle_size+1),MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);//wait for the previous part of board to finish
			for(i=0;i<sub_size;i++){
				out=fopen(out_file,"a");
				fprintf(out,"\n");
				for(j=0;j<sub_size;j++){
					fprintf(out, "%c", board[i][j] );
				}
				fclose(out);
				MPI_Send(&start,1,MPI_BYTE,my_rank+1,RIGHT,MPI_COMM_WORLD);//dwse skutalh ston epomeno
				MPI_Recv(&start,1,MPI_BYTE,my_rank+cycle_size,MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);//perimene apo to ton teleutaio
			}
//			MPI_Send(&start,1,MPI_BYTE,my_rank+cycle_size+1,my_rank,MPI_COMM_WORLD);//epomenh seira
			printf("(%d) THE END IS HERE EVYRITHING WRITE CORRECT\n",my_rank);
		}else if( (my_rank)%(cycle_size+1)==0 ){//an einai prwto sthn seira
			MPI_Recv(&start,1,MPI_BYTE,my_rank-(cycle_size+1),MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);//wait for the previous part of board to finish
			for(i=0;i<sub_size;i++){
				out=fopen(out_file,"a");
				fprintf(out,"\n");
				for(j=0;j<sub_size;j++){
					fprintf(out, "%c", board[i][j] );
				}
				fclose(out);
				MPI_Send(&start,1,MPI_BYTE,my_rank+1,RIGHT,MPI_COMM_WORLD);//dwse skutalh ston epomeno
				MPI_Recv(&start,1,MPI_BYTE,my_rank+cycle_size,MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);//perimene apo to ton teleutaio
			}
			MPI_Send(&start,1,MPI_BYTE,my_rank+cycle_size+1,my_rank,MPI_COMM_WORLD);//epomenh seira
		}else if( (my_rank+1)%(cycle_size+1)==0 ){//an einai teleutaio
			for(i=0;i<sub_size;i++){
				MPI_Recv(&start,1,MPI_BYTE,my_rank-1,MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);//perimene apo to ton prohgoumeno t
				out=fopen(out_file,"a");
				for(j=0;j<sub_size;j++){
					fprintf(out, "%c", board[i][j] );
				}
				fclose(out);
				MPI_Send(&start,1,MPI_BYTE,my_rank-cycle_size,RIGHT,MPI_COMM_WORLD);//dwse skutalh ston epomeno
			}
		}else{
			for(i=0;i<sub_size;i++){
				MPI_Recv(&start,1,MPI_BYTE,my_rank-1,MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);//perimene apo to ton teleutaio
				out=fopen(out_file,"a");
				for(j=0;j<sub_size;j++){
					fprintf(out, "%c", board[i][j] );
				}
				fclose(out);
				MPI_Send(&start,1,MPI_BYTE,my_rank+1,RIGHT,MPI_COMM_WORLD);//dwse skutalh ston epomeno
			}
		}
	}
	printf("(%d) END WITH THE WRITTING\n", my_rank);
#endif
	MPI_Finalize();
	return 0;
}
