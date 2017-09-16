#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define ALIVE 'X'
#define DEAD 'O'
#define DEBUG 1

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
