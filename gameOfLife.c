#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ALIVE 'X'
#define DEAD 'O'
#define DEBUG 1

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
    int my_coord[2];// my i,j in the cartesian topologie
    int coord[2];
    int return_val;

    int number_of_process,dimensions,my_rank,blocks_per_line;
    int blockDimension;
    double tempblocks_per_line;

    FILE *fp;

    // 8 variables for the ids of each neighbor
    int up_left_id,up_id,up_right;
    int left_id,right_id;
    int down_left_id,down_id,down_right_id;

    srand ( 0 );
    fp=fopen("input.txt","r");
    if(fp==NULL){
        perror("Fail to open the file");
    }
    fscanf(fp, "%d",&dimensions);


    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_process);
    tempblocks_per_line=sqrt(number_of_process);
    blocks_per_line=(int) tempblocks_per_line;
    blockDimension=dimensions/blocks_per_line;

    MPI_Datatype  oneRow , oneCol;
    // it's 9 blocks     one element in each block    number of elements beetwen blocks
    MPI_Type_vector(8, 1, blockDimension, MPI_INT , &oneCol );
    MPI_Type_vector(8, 1, 1, MPI_INT , &oneRow );
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

    MPI_Barrier( MPI_COMM_WORLD);


    //initialize the block
    char** block;
    char** newblock;
    char** empty_block;
    int random_number;
    block=malloc(sizeof(char*)*blockDimension);
    newblock=malloc(sizeof(char*)*blockDimension);
    empty_block=malloc(sizeof(char*)*blockDimension);
    for(i=0;i<blockDimension;i++){
        block[i]=malloc(sizeof(char)*blockDimension);
        newblock[i]=malloc(sizeof(char)*blockDimension);
        empty_block[i]=malloc(sizeof(char)*blockDimension);
    }
    //take random vars
    for(i=0;i<blockDimension;i++){
        for(j=0;j<blockDimension;j++){
            random_number = rand()%10;
            empty_block[i][j]='q';
            if(random_number==0){
                block[i][j]=ALIVE;
            }else{
                block[i][j]=DEAD;
            }
        }
    }

    if(my_rank==0){
        print_board(block,blockDimension,stdout);
        fflush(stdout);
    }

    MPI_Barrier( MPI_COMM_WORLD);
    if(my_rank==0){
        MPI_Send(&block[1][0], 1, oneCol , right_id , blockDimension+1 , cartesian_comm );//send of the first col
        // MPI_Send(&block[0][1], 1, oneRow , right_id , 11 , cartesian_comm );//send of the first row
    }
    if(left_id==0){
        MPI_Recv(&empty_block[1][0],1,oneCol,left_id,blockDimension+1,cartesian_comm,MPI_STATUS_IGNORE);
        // MPI_Recv(&empty_block[0][1],1,oneRow,left_id,11,cartesian_comm,MPI_STATUS_IGNORE);
        printf("\nrecieved\n");
        fflush(stdout);
        print_board(empty_block, blockDimension, stdout);
        printf("\n end \n");
        fflush(stdout);
    }

    //calculate the inside
    int neighbors=0;
    // printf("My first update\n");
    for(i=1;i<blockDimension-1;i++){
        for(j=1;j<blockDimension-1;j++){
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
                    newblock[i][j]=DEAD;
                }else if(neighbors==2 || neighbors==3){
                    newblock[i][j]=ALIVE;
                }else if(neighbors>3){
                    newblock[i][j]=DEAD;
                }
            }else{
                if(neighbors==3){
                    newblock[i][j]=ALIVE;
                }else{
                    newblock[i][j]=DEAD;
                }
            }
        }
    }


    // if(my_rank==0){
    //     printf("\n\n");
    //     print_board(block,blockDimension,stdout);
    //     printf("\n\n");
    //     print_board_inside(newblock,blockDimension,stdout);
    // }


    for(i=0;i<blockDimension;i++){
        free(block[i]);
    }
    free(block);
    fclose(fp);
    MPI_Finalize();
    return 0;
}
