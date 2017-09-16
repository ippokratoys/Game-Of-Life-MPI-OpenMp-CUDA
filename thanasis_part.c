#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define ALIVE 'X'
#define DEAD 'O'
#define DEBUG 1
void print_board(int** board,int size,FILE* stream){
    int i,j;
    for ( i = 0; i < size; i++) {
        for ( j = 0; j < size; j++) {
            fprintf(stream,"%3d ", board[i][j]);
            // putc(board[i][j], stream);
        }
        putc('\n', stream);
    }

}

int main(int argc, char  *argv[]) {
    int retun_val;//what fun returns

    int my_coord[2];// my i,j in the cartesian topologie
    int my_rank;

    // 8 variables for the ids of each neighbor
    int up_left_id,up_id,up_right;
    int left_id,right_id;
    int down_left_id,down_id,down_right_id;

    int number_of_process;
    MPI_Init(&argc,&argv);


// check how many process we have
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_process);
    int blocks_per_line = sqrt(number_of_process);

    int coord[2];

//create the datypes for send/recv the matrix
    MPI_Datatype  oneRow , oneCol;
    // it's 9 blocks     one element in each block    number of elements beetwen blocks
    MPI_Type_vector(8, 1, 10, MPI_INT , &oneCol );
    MPI_Type_vector(8, 1, 1, MPI_INT , &oneRow );
    MPI_Type_commit (&oneRow);
    MPI_Type_commit (&oneCol);


//bulit the new cartesian communicator
    MPI_Comm cartesian_comm;//the communicator we are going to use
    int dim[2];//calculate the dimensions
    dim[0]=dim[1]=blocks_per_line;//it's NxN
    int period[]={1,1};//it's periodican both vertical and horizontal
    int reorder = 1;//reorder the process if nessesary

    //create the topologie
    retun_val = MPI_Cart_create(MPI_COMM_WORLD,2,dim,period,reorder,&cartesian_comm);
    if (retun_val!=0 ){
      fprintf(stderr, "error creating communicator %d\n",retun_val);
      MPI_Abort(MPI_COMM_WORLD, retun_val);
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
    MPI_Barrier( MPI_COMM_WORLD);
    int** board=malloc(sizeof(int*)*10);
    board[0]=malloc(sizeof(int)*10*10);

    int** empty_board=malloc(sizeof(int*)*10);
    empty_board[0]=malloc(sizeof(int)*10*10);
    int i,j;

    for ( i = 0; i < 10; i++) {
        board[i]=&board[0][i*10];
        empty_board[i]=&empty_board[0][i*10];
    }

    for ( i = 0; i < 10; i++) {
        // board[i]=malloc(sizeof(int)*10);
        // empty_board[i]=malloc(sizeof(int)*10);
        for ( j = 0; j < 10; j++) {
            board[i][j]=i*10+j;
            empty_board[i][j]=-1;
        }
    }
    if(my_rank==0){
        print_board(board,10,stdout);
        fflush(stdout);
    }

    MPI_Barrier( MPI_COMM_WORLD);
    if(my_rank==0){
        MPI_Send(&board[1][0], 1, oneCol , right_id , 11 , cartesian_comm );//send of the first col
        // MPI_Send(&board[0][1], 1, oneRow , right_id , 11 , cartesian_comm );//send of the first row
    }
    if(left_id==0){
        MPI_Recv(&empty_board[1][0],1,oneCol,left_id,11,cartesian_comm,MPI_STATUS_IGNORE);
        // MPI_Recv(&empty_board[0][1],1,oneRow,left_id,11,cartesian_comm,MPI_STATUS_IGNORE);
        printf("\nrecieved\n");
        fflush(stdout);
        print_board(empty_board, 10, stdout);
        printf("\n end \n");
        fflush(stdout);
    }
#endif
// my_coord[2]       : this process coordinates
// ***_id            : the id of the *** process is computed
// my_rank           : the rank of the current process
// blocks_per_line   : how many blocks we have per line/column
// cartesian_comm    : the communicator we will use from now on
// number_of_process : how many process we have
    MPI_Finalize();
    return 0;
}
