#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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


int main(int argc, char  *argv[]) {
    int i,j;
    int world_size,dimensions,myrank,BlocksColAndRows;
    int mycolumn,myrow,blockDimension;
    double tempBlocksColAndRows;
    FILE *fp;
    srand ( 0 );


    fp=fopen("input.txt","r");
    if(fp==NULL){
        perror("Fail to open the file");
    }
    fscanf(fp, "%d",&dimensions);


      MPI_Init(&argc,&argv);
      MPI_Comm_size(MPI_COMM_WORLD, &world_size);
      tempBlocksColAndRows=sqrt(world_size);
      BlocksColAndRows=(int) tempBlocksColAndRows;
      blockDimension=dimensions/BlocksColAndRows;

      MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

      //find my column and my row
      if(myrank==0){
          // printf("Num of Processes:%d\nNum of dimensions:%d\nBlocks rows and columns:%d\nEach Block is %dX%d\n",world_size,dimensions,BlocksColAndRows,blockDimension,blockDimension);
          mycolumn=myrow=0;
      }else{
        int tempcol=0,temprow=0;
        for(i=0;i<world_size;i++){
            if(i==myrank){
                mycolumn=tempcol;
                myrow=temprow;
                break;
            }
            tempcol++;
            if(tempcol==BlocksColAndRows){
                tempcol=0;
                temprow++;
                continue;
            }
        }
    }
    char* message="GEIA";
    MPI_Request request;
    if(myrank+1<world_size){
        MPI_Isend(message, 5, MPI_CHAR, (myrank+1), 13,MPI_COMM_WORLD, &request);
    }

    if(myrank!=0){
        char* in;
        in=malloc(sizeof(char)*5);
        MPI_Request recv_status;
        MPI_Status status;
        MPI_Irecv(in, 5, MPI_CHAR, (myrank-1), 13, MPI_COMM_WORLD, &recv_status);
        MPI_Wait(&recv_status,&status);
        printf("%s\n",in);
    }

    // printf("My rank is %d:\nMy column is %d My row is %d \n",myrank,mycolumn,myrow );

    //initialize the block
    char** block;
    char** newblock;
    int random_number;
    block=malloc(sizeof(char*)*blockDimension);
    newblock=malloc(sizeof(char*)*blockDimension);
    for(i=0;i<blockDimension;i++){
        block[i]=malloc(sizeof(char)*blockDimension);
        newblock[i]=malloc(sizeof(char)*blockDimension);
    }
    //take random vars
    for(i=0;i<blockDimension;i++){
        for(j=0;j<blockDimension;j++){
            random_number = rand()%10;
            if(random_number==0){
                block[i][j]=ALIVE;
            }else{
                block[i][j]=DEAD;
            }
        }
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
    if(myrank==0){
        printf("\n\n");
        print_board(block,blockDimension,stdout);
        printf("\n\n");
        print_board(newblock,blockDimension,stdout);
    }


    for(i=0;i<blockDimension;i++){
        free(block[i]);
    }
    free(block);
    fclose(fp);
    MPI_Finalize();
    return 0;
}
