#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define ALIVE 'X'
#define DEAD 'O'

void print_board(char** board,int size,FILE* stream){
    int i,j;
    for ( i = 0; i < size; i++) {
        for ( j = 0; j < size; j++) {
            putc(board[i][j], stream);
        }
        putc('\n', stream);
    }

}

double sqroot(double square)
{
    double root=square/3;
    int i;
    if (square <= 0) return 0;
    for (i=0; i<32; i++)
        root = (root + square / root) / 2;
    return root;
}

int main(int argc, char  *argv[]) {
    int i,j;
    int world_size,dimensions,myrank,BlocksColAndRows;
    int mycolumn,myrow,blockDimension;
    double tempBlocksColAndRows;
    int retun_val;
    FILE *fp;

    srand ( 0 );


    fp=fopen("input.txt","r");
    if(fp==NULL){
        perror("Fail to open the file");
    }
    fscanf(fp, "%d",&dimensions);


    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    tempBlocksColAndRows=sqroot(world_size);
    BlocksColAndRows=(int) tempBlocksColAndRows;
    blockDimension=dimensions/BlocksColAndRows;

    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    // create the communicator
    MPI_Comm cartesian_comm;
    int dim[2];//calculate the dimensions
    int blocks_per_line = (int) sqroot(world_size);
    dim[0]=dim[1]=blocks_per_line;//it's NxN
    int period[]={1,1};//it's periodican both vertical and horizontal
    int reorder = 1;
    retun_val = MPI_Cart_create(MPI_COMM_WORLD,2,dim,period,reorder,&cartesian_comm);
    if (retun_val!=0 ){
      fprintf(stderr, "error creating communicator %d\n",retun_val);
      MPI_Abort(MPI_COMM_WORLD, retun_val);
    }
    int new_rank;
    // MPI_Comm_rank(cartesian_comm,&new_rank);
    int coord[2];
    // MPI_Cart_coords(cartesian_comm, myrank, 2, coord);
    // printf("Rank %2d coordinates are %1d %1d\n", myrank, coord[0], coord[1]);fflush(stdout);

    coord[0]=16; coord[1]=16;int id;
    MPI_Cart_rank(cartesian_comm, coord, &id);
    printf("The processor at position (%d, %d) has rank %d\n", coord[0], coord[1], id);fflush(stdout);

    // printf("%d -> %d\n",myrank,new_rank);
    //find my column and my row
    if(myrank==0){
        // printf("Num of Processes:%d\nNum of dimensions:%d\nBlocks rows and columns:%d\nEach Block is %dX%d\n",world_size,dimensions,BlocksColAndRows,blockDimension,blockDimension);
        mycolumn=myrow=0;
    }
    // else{
    //   int tempcol=0,temprow=0;
    //   for(i=0;i<world_size;i++){
    //     if(i==myrank){
    //       mycolumn=tempcol;
    //       myrow=temprow;
    //       break;
    //     }
    //     tempcol++;
    //     if(tempcol==BlocksColAndRows){
    //       tempcol=0;
    //       temprow++;
    //       continue;
    //     }
    //   }
    // }
    // // printf("My rank is %d:\nMy column is %d My row is %d \n",myrank,mycolumn,myrow );
    //
    // //initialize the block
    // char** block;
    // char** change;
    // int random_number;
    // // allocate memory for the block +1 line that we will recieve
    // block=malloc(sizeof(char*)*(blockDimension+1));
    // change=malloc(sizeof(char*)*(blockDimension+1));
    // for(i=0;i<blockDimension;i++){
    //   //   same here +1 column
    //   block[i]=malloc(sizeof(char)*(blockDimension+1));
    //   change[i]=malloc(sizeof(char)*(blockDimension+1));
    // }
    // //take random vars
    // for(i=1;i<blockDimension;i++){
    //   for(j=1;j<blockDimension;j++){
    //     random_number = rand()%10;
    //     change[i][j]='N';
    //     if(random_number==0){
    //       block[i][j]=ALIVE;
    //     }else{
    //       block[i][j]=DEAD;
    //     }
    //   }
    // }
    //
    // //calculate the inside
    // int neighbors=0,changed=0;
    // printf("My first update\n");
    // for(i=2;i<blockDimension-1;i++){
    //   if(myrank==0){
    //     printf("%s\n",block[i]);
    //   }
    //   for(j=2;j<blockDimension-1;j++){
    //       neighbors=0;
    //       changed=0;
    //       if(block[i-1][j-1]==ALIVE){
    //         neighbors++;
    //       }else if(block[i-1][j]==ALIVE){
    //         neighbors++;
    //       }else if(block[i-1][j+1]==ALIVE){
    //         neighbors++;
    //       }else if(block[i][j-1]==ALIVE){
    //         neighbors++;
    //       }else if(block[i][j+1]==ALIVE){
    //         neighbors++;
    //       }else if(block[i+1][j-1]==ALIVE){
    //         neighbors++;
    //       }else if(block[i+1][j]==ALIVE){
    //         neighbors++;
    //       }else if(block[i+1][j+1]==ALIVE){
    //         neighbors++;
    //       }
    //       if(block[i][j]==ALIVE){
    //         changed=1;
    //         if(neighbors<=1){
    //           change[i][j]='D';
    //         }else if(neighbors==2 || neighbors==3){
    //           change[i][j]='L';
    //         }else if(neighbors>3){
    //           change[i][j]='D';
    //         }
    //       }else{
    //         if(neighbors==3){
    //           changed=1;
    //           change[i][j]='L';
    //         }
    //       }
    //       if(changed==0)change[i][j]='N';
    //   }
    // }
    // for(i=1;i<blockDimension-1;i++){
    //   for(j=1;j<blockDimension-1;j++){
    //     if(change[i][j]=='D'){
    //       block[i][j]=DEAD;
    //     }else if(change[i][j]=='L'){
    //       block[i][j]=ALIVE;
    //     }
    //   }
    // }
    // printf("\n\n");
    // if(myrank==0){
    //     print_board(block,blockDimension,stdout);
    // }
    //
    // for(i=0;i<blockDimension+1;i++){
    //   free(block[i]);
    // }
    // free(block);
    // fclose(fp);
    MPI_Finalize();
    return 0;
}
