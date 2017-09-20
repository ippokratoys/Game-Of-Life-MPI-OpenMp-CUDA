#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ALIVE 'X'
#define DEAD '.'
#define EMPTY '?'
#define DEBUG 1
//how many loops will hapen
#define TOTAL_LOOPS 1
//every how many loops we have to check for a change (if 0 no check at all)
#define CHECK_FOR_CHANGE 0

void print_board(char** board,int size,FILE* stream){
    int i,j;
    for ( i = 0; i < size; i++) {
        for ( j = 0; j < size; j++) {
            putc(board[i][j], stream);
        }
        putc('\n', stream);
    }

}

void print_board_inside(char** board,int size,FILE* stream){
    int i,j;
    for ( i = 1; i < size-1; i++) {
        for ( j = 1; j < size-1; j++) {
            putc(board[i][j], stream);
        }
        putc('\n', stream);
    }

}

int main(int argc, char  *argv[]) {
    int i,j;
    int my_coord[2];// process i,j in the cartesian topologie
    int coord[2];//buffer coordinates for finding neighbors
    int return_val;//buffer

    int number_of_process,//number of process
        dimensions,//dimensions of the (full)grid, read by the file
        my_rank,//my rank in the cartesian  topologie
        blocks_per_line;//how many blocks we have in each line/row
    int blockDimension;//the dimension of each process block
    double tempblocks_per_line;

    FILE *fp;//the file from wich we will do the reading

    // 8 variables for the ids of each neighbor
    int up_left_id,up_id,up_right;
    int left_id,right_id;
    int down_left_id,down_id,down_right_id;

    fp=fopen("input.txt","r");
    if(fp==NULL){
        perror("Fail to open the file");
    }
    //read the grid from the file
    fscanf(fp, "%d",&dimensions);


    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_process);
    //do the basic calculation for the size of the grid
    tempblocks_per_line=sqrt(number_of_process);
    blocks_per_line=(int) tempblocks_per_line;
    blockDimension=dimensions/blocks_per_line;

    MPI_Datatype  oneRow , oneCol;
    // it's 9 blocks     one element in each block    number of elements beetwen blocks
    MPI_Type_vector(blockDimension, 1, blockDimension+2, MPI_CHAR  , &oneCol );
    MPI_Type_vector(blockDimension, 1, 1, MPI_CHAR  , &oneRow );
    MPI_Type_commit (&oneRow);
    MPI_Type_commit (&oneCol);

    MPI_Comm cartesian_comm;//the communicator we are going to use
    int dim[2];//calculate the dimensions
    dim[0]=dim[1]=blocks_per_line;//it's NxN
    int period[]={1,1};//it's periodican both vertical and horizontal
    int reorder = 1;//reorder the process if nessesary

    //create the topologie
    return_val = MPI_Cart_create(MPI_COMM_WORLD,2,dim,period,reorder,&cartesian_comm);
    if (return_val!=0 ){
        fprintf(stderr, "error creating communicator %d\n",return_val);
        MPI_Abort(MPI_COMM_WORLD, return_val);
    }
    //get my ranks
    MPI_Comm_rank(cartesian_comm,&my_rank);//find my rank in this communicator
    MPI_Cart_coords(cartesian_comm, my_rank, 2, my_coord);//and find my position on the grid
    srand ( my_rank+23 );

#if DEBUG==1
    printf("Rank %2d coordinates are %1d %1d\n", my_rank, my_coord[0], my_coord[1]);fflush(stdout);
#endif

    //foud my neighbors id's
    //up line
    coord[0]=my_coord[0]-1;
    coord[1]=my_coord[1]-1;
    MPI_Cart_rank(cartesian_comm, coord , &up_left_id);

    coord[0]=my_coord[0]-1;
    coord[1]=my_coord[1];
    MPI_Cart_rank(cartesian_comm, coord , &up_id);

    coord[0]=my_coord[0]-1;
    coord[1]=my_coord[1]+1;
    MPI_Cart_rank(cartesian_comm, coord , &up_right);
    //same line
    coord[0]=my_coord[0];
    coord[1]=my_coord[1]-1;
    MPI_Cart_rank(cartesian_comm, coord , &left_id);

    coord[0]=my_coord[0];
    coord[1]=my_coord[1]+1;
    MPI_Cart_rank(cartesian_comm, coord , &right_id);

    //down line
    coord[0]=my_coord[0]+1;
    coord[1]=my_coord[1]-1;
    MPI_Cart_rank(cartesian_comm, coord , &down_left_id);

    coord[0]=my_coord[0]+1;
    coord[1]=my_coord[1];
    MPI_Cart_rank(cartesian_comm, coord , &down_id);

    coord[0]=my_coord[0]+1;
    coord[1]=my_coord[1]+1;
    MPI_Cart_rank(cartesian_comm, coord , &down_right_id);

#if DEBUG==1

    //random print of someones neighobrs just for check
    if(my_rank==0){
        printf("up_left_id    %d\n", up_left_id);
        printf("up_id         %d\n", up_id);
        printf("left_id       %d\n", left_id);
        printf("down_left_id  %d\n", down_left_id);
        printf("down_id       %d\n", down_id);
        printf("up_right      %d\n", up_right);
        printf("right_id      %d\n", right_id);
        printf("down_right_id %d\n", down_right_id);
        fflush(stdout);
    }
#endif

    MPI_Barrier( MPI_COMM_WORLD);//wait for all process to reach here


    //initialize the block
    char** block;
    char** newblock;
    int random_number;
    block=malloc(sizeof(char*)*(blockDimension+2));
    newblock=malloc(sizeof(char*)*(blockDimension+2));
    //we are allocating memomory in a special way, think about it
    block[0]=malloc(sizeof(char)*(blockDimension+2)*(blockDimension+2));
    newblock[0]=malloc(sizeof(char)*(blockDimension+2)*(blockDimension+2));
    for(i=0;i<blockDimension+2;i++){
        block[i]=&block[0][i*(blockDimension+2)];
        newblock[i]=&newblock[0][i*(blockDimension+2)];
    }
    //take random vars

    //initialize the board's with 'q'
    for(i=0;i<blockDimension+2;i++){
        for(j=0;j<blockDimension+2;j++){
            block[i][j]=EMPTY;
            newblock[i][j]=EMPTY;
        }
    }
//random initialazation of the grid in each process
    // for(i=1;i<blockDimension+1;i++){
    //     for(j=1;j<blockDimension+1;j++){
    //         random_number = rand()%10;
    //         if(random_number==0){
    //             block[i][j]=ALIVE;
    //         }else{
    //             block[i][j]=DEAD;
    //         }
    //     }
    // }
//initialazation form the file
////////////////do the reading////////////////
    int bufc;
    int cycle_size=dimensions/blockDimension;// CAREFULLL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //how much i must move in the file for one line
    int file_line_offset=dimensions+1;//because of the \n
    int first_line=(my_rank/cycle_size)*(blockDimension);
	int first_col=(my_rank%cycle_size)*(blockDimension);
    int correct_offset=file_line_offset-blockDimension;

	int seek;
	seek=1+file_line_offset*first_line;//move to the correct line
	seek+=first_col;//move to the correct column
	fseek(fp,seek,SEEK_CUR);
	for(i=first_line;i<first_line+blockDimension;i++){
		//offset move in the file correct line and row
		for(j=0;j<blockDimension;j++){
			bufc=fgetc(fp);
//			if(j==0)printf("rank:%d char:|%c|\n",my_rank,bufc);
//			if(bufc!=ALIVE && bufc!=DEAD )printf("error reading|%c|\n",bufc);
			if(bufc!=DEAD && bufc!=ALIVE){
				printf("LOOOL rank:%d-i=%d,j=%d,it's |%c|\n",my_rank,i,j,bufc);
			}
			block[i-first_line+1][j+1]=bufc;
		}//be carefull of the \n
		fseek(fp,correct_offset,SEEK_CUR);//go to the next line
	}
	printf("!%d end with reading\n",my_rank);
    fflush(stdout);
    // MPI_Barrier( MPI_COMM_WORLD);
	fclose(fp);//close the file of input
////////////////end of reading////////////////



    // sleep(1);
    // if(my_rank==number_of_process-1){
    //     print_board(block,blockDimension+2,stdout);
    //     fflush(stdout);
    // }
    // MPI_Barrier( MPI_COMM_WORLD);
    // exit(11);


    int cur_loop = 0;
    int changed = 0;//a variable to know if somthing has change
    MPI_Barrier( MPI_COMM_WORLD);
    int total_changes;//total changes in one loop
    for (cur_loop = 0; cur_loop < TOTAL_LOOPS; cur_loop++) {
        total_changes=0;
        changed=0;//nothing has change
        MPI_Request send_requests[8];
        MPI_Isend(&block[1][1], 1, oneCol , left_id , blockDimension+1 , cartesian_comm,&send_requests[0] );//send of the first col
        MPI_Isend(&block[1][blockDimension],1,oneCol,right_id,blockDimension+1,cartesian_comm,&send_requests[1]);//send of the last col
        MPI_Isend(&block[blockDimension][1],1,oneRow,down_id,blockDimension+1,cartesian_comm,&send_requests[2]);//send of the last line
        MPI_Isend(&block[1][1],1,oneRow,up_id,blockDimension+1,cartesian_comm,&send_requests[3]);//send of the first line
        MPI_Isend(&block[1][1],1,MPI_CHAR,up_left_id,1,cartesian_comm,&send_requests[4]);//send of the up left
        MPI_Isend(&block[1][blockDimension],1,MPI_CHAR,up_right,1,cartesian_comm,&send_requests[5]);//send of the up right
        MPI_Isend(&block[blockDimension][blockDimension],1,MPI_CHAR,down_right_id,1,cartesian_comm,&send_requests[6]);//send of the down right
        MPI_Isend(&block[blockDimension][1],1,MPI_CHAR,down_left_id,1,cartesian_comm,&send_requests[7]);//send of the down left

        // sleep(2);

        MPI_Recv(&block[1][0],1,oneCol,left_id,blockDimension+1,cartesian_comm,MPI_STATUS_IGNORE);//receive of first column
        MPI_Recv(&block[1][blockDimension+1],1,oneCol,right_id,blockDimension+1,cartesian_comm,MPI_STATUS_IGNORE);//recieve of the last column
        MPI_Recv(&block[0][1],1,oneRow,up_id,blockDimension+1,cartesian_comm,MPI_STATUS_IGNORE);//recieve of the firt line
        MPI_Recv(&block[blockDimension+1][1],1,oneRow,down_id,blockDimension+1,cartesian_comm,MPI_STATUS_IGNORE);//recieve of the last line
        MPI_Recv(&block[blockDimension+1][blockDimension+1],1,MPI_CHAR,down_right_id,1,cartesian_comm,MPI_STATUS_IGNORE);//receive of the down right
        MPI_Recv(&block[blockDimension+1][0],1,MPI_CHAR,down_left_id,1,cartesian_comm,MPI_STATUS_IGNORE);//recieve of the down left
        MPI_Recv(&block[0][blockDimension+1],1,MPI_CHAR,up_right,1,cartesian_comm,MPI_STATUS_IGNORE);//recieve of the up right
        MPI_Recv(&block[0][0],1,MPI_CHAR,up_left_id,1,cartesian_comm,MPI_STATUS_IGNORE);//recieve fo the up left

        if(my_rank==0){
            printf("\n\n");
            print_board(block,blockDimension+2,stdout);
            fflush(stdout);
        }

        //calculate the inside
        int neighbors=0;
        // printf("My first update\n");
        for(i=2;i<blockDimension;i++){
            for(j=2;j<blockDimension;j++){
                neighbors=0;
                if(block[i-1][j-1]==ALIVE)neighbors++;
                if(block[i-1][j]==ALIVE)neighbors++;
                if(block[i-1][j+1]==ALIVE)neighbors++;
                if(block[i][j-1]==ALIVE)neighbors++;
                if(block[i][j+1]==ALIVE)neighbors++;
                if(block[i+1][j-1]==ALIVE)neighbors++;
                if(block[i+1][j]==ALIVE)neighbors++;
                if(block[i+1][j+1]==ALIVE)neighbors++;
                if(block[i][j]==ALIVE){
                    if(neighbors<=1){
                        changed++;
                        newblock[i][j]=DEAD;
                    }else if(neighbors==2 || neighbors==3){
                        newblock[i][j]=ALIVE;
                    }else if(neighbors>3){
                        changed++;
                        newblock[i][j]=DEAD;
                    }
                }else{
                    if(neighbors==3){
                        changed++;
                        newblock[i][j]=ALIVE;
                    }else{
                        newblock[i][j]=DEAD;
                    }
                }
            }
        }

        //updates the outer part
        //the i takes just two values: 1 , blockDimension
        //the j takes all the values from [1,blockDimension]
        for(i=1;i<blockDimension+1;i+=blockDimension-1){

            //update the first and last row
            for(j=1;j<blockDimension+1;j++){
                neighbors=0;
                if(block[i-1][j-1]==ALIVE)neighbors++;
                if(block[i-1][j]==ALIVE)neighbors++;
                if(block[i-1][j+1]==ALIVE)neighbors++;
                if(block[i][j-1]==ALIVE)neighbors++;
                if(block[i][j+1]==ALIVE)neighbors++;
                if(block[i+1][j-1]==ALIVE)neighbors++;
                if(block[i+1][j]==ALIVE)neighbors++;
                if(block[i+1][j+1]==ALIVE)neighbors++;

                if(block[i][j]==ALIVE){
                    if(neighbors<=1){
                        changed++;
                        newblock[i][j]=DEAD;
                    }else if(neighbors==2 || neighbors==3){
                        newblock[i][j]=ALIVE;
                    }else if(neighbors>3){
                        changed++;
                        newblock[i][j]=DEAD;
                    }
                }else{
                    if(neighbors==3){
                        changed++;
                        newblock[i][j]=ALIVE;
                    }else{
                        newblock[i][j]=DEAD;
                    }
                }
            }

            //now we do the same, but instead of having the i fixed we have the j add we use [j][i]
            //this updates the first and last column
            //you don't actual need a second loop, you can just add it to the up loop
            //(check the corerners twice, not a problem)
            for(j=1;j<blockDimension+1;j++){
                neighbors=0;
                if(block[j-1][i-1]==ALIVE)neighbors++;
                if(block[j-1][i]==ALIVE)neighbors++;
                if(block[j-1][i+1]==ALIVE)neighbors++;
                if(block[j][i-1]==ALIVE)neighbors++;
                if(block[j][i+1]==ALIVE)neighbors++;
                if(block[j+1][i-1]==ALIVE)neighbors++;
                if(block[j+1][i]==ALIVE)neighbors++;
                if(block[j+1][i+1]==ALIVE)neighbors++;

                if(block[j][i]==ALIVE){
                    if(neighbors<=1){
                        changed++;
                        newblock[j][i]=DEAD;
                    }else if(neighbors==2 || neighbors==3){
                        newblock[j][i]=ALIVE;
                    }else if(neighbors>3){
                        changed++;
                        newblock[j][i]=DEAD;
                    }
                }else{
                    if(neighbors==3){
                        changed++;
                        newblock[j][i]=ALIVE;
                    }else{
                        newblock[j][i]=DEAD;
                    }
                }
            }
        }

        if(my_rank==0){
            printf("\n\nThe New Block of 0\n\n");
            print_board(newblock,blockDimension+2,stdout);
            printf("\n\n");
        }
        if(my_rank!=0){
            printf("(%2d)%2d : %3d\n",cur_loop, my_rank,changed);
        }

//no need to add this code if the check is 0
#if CHECK_FOR_CHANGE>0
        //if in this loop we have to check for change
        if(cur_loop%CHECK_FOR_CHANGE==0){
            MPI_Allreduce(&changed,&total_changes,1,MPI_INT,MPI_SUM,cartesian_comm);
            if(my_rank==0){
                printf("Total changes : %d\n",total_changes);
                if(total_changes==0){
                    printf("Nooothing chaned on loop %d/%d\n",cur_loop+1,TOTAL_LOOPS);
                }
            }
            if(total_changes==0){
                break;
            }
        }
#endif
        //swap the newblock with the block variable
        char ** temp;
        temp=newblock;
        newblock=block;
        block=temp;
    }
//write the output to a file
    //take the needed part of the board(not the ghost-edges)
    char** final_board=malloc(sizeof(char*)*blockDimension);
    int is_last = 0 ;
    if( my_coord[1]+1==blocks_per_line ){
        is_last=1;//if it's in the last line we need t oadd \n
    }
    final_board[0]=malloc(sizeof(char)*(blockDimension*blockDimension +is_last*blockDimension));
    //do the malloc
    if(is_last==1){
        //if it's the last one
        for (i = 0; i < blockDimension; i++) {
            final_board[i]=&final_board[0][i*(blockDimension+1)];
            final_board[i][blockDimension]='\n';//and add as last elemt the new line
        }
    }else{
        for (i = 0; i < blockDimension; i++) {
            final_board[i]=&final_board[0][i*blockDimension];
        }
    }

    //copy the board
    for (i = 0; i < blockDimension; i++) {
        for (j = 0; j < blockDimension; j++) {
            final_board[i][j]=block[i+1][j+1];
        }
    }
    MPI_Datatype my_sub_array;//the datatype we are going to create
    int starts[2];//where this arays starts from
    int sub_size[2]  = {blockDimension, blockDimension};
    int big_size[2]  = {dimensions, dimensions+1};//it's plus one the new line
    starts[0]=my_coord[0]*blockDimension;
    starts[1]=my_coord[1]*blockDimension;
    if(is_last==1){
        sub_size[1]++;//the sub size is 1 bigger
    }
    MPI_Barrier(cartesian_comm);
    MPI_Type_create_subarray(2, big_size, sub_size, starts,MPI_ORDER_C, MPI_CHAR, &my_sub_array);
    MPI_Type_commit (&my_sub_array);
    MPI_File   file;//the file that we will use for output
    MPI_Status file_status;
    MPI_Status write_status;
    MPI_File_open(cartesian_comm, "out.txt",MPI_MODE_CREATE|MPI_MODE_WRONLY,MPI_INFO_NULL, &file);
    if(my_rank==0){
        char buf[1000];
        sprintf(buf, "%010d\n",dimensions);
        MPI_File_write(file, buf, 11,MPI_CHAR, &file_status);
    }
    MPI_File_set_view(file, 11,  MPI_CHAR, my_sub_array,"native", MPI_INFO_NULL);

    if(is_last==1){
        MPI_File_write_all(file, final_board[0],blockDimension*blockDimension+blockDimension, MPI_CHAR, &write_status);
    }else{
        MPI_File_write_all(file, final_board[0],blockDimension*blockDimension , MPI_CHAR, &write_status);
    }
    MPI_File_close(&file);
// end of file output
    // for(i=0;i<blockDimension;i++){
    //     free(block[i]);
    // free(block);
    // }
    MPI_Finalize();
    return 0;
}
