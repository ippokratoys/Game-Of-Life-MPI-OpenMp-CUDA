#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
  FILE *fp;

  srand ( time(NULL) );


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

  //find my column and my row
  if(myrank==0){
    printf("Num of Processes:%d\nNum of dimensions:%d\nBlocks rows and columns:%d\nEach Block is %dX%d\n",world_size,dimensions,BlocksColAndRows,blockDimension,blockDimension);
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
  printf("My rank is %d:\nMy column is %d My row is %d \n",myrank,mycolumn,myrow );

  //initialize the block
  char** block;
  char** change;
  int random_number;
  block=malloc(sizeof(char*)*blockDimension);
  change=malloc(sizeof(char*)*blockDimension);
  for(i=0;i<blockDimension;i++){
    block[i]=malloc(sizeof(char)*blockDimension);
    change[i]=malloc(sizeof(char)*blockDimension);
  }
  //take random vars
  for(i=0;i<blockDimension;i++){
    for(j=0;j<blockDimension;j++){
      random_number = rand()%2;
      change[i][j]='N';
      if(random_number==0){
        block[i][j]='O';
      }else{
        block[i][j]='X';
      }
    }
  }

  //calculate the inside
  int neighbors=0,changed=0;
  for(i=1;i<blockDimension-1;i++){
    for(j=1;j<blockDimension-1;j++){
        if(block[i-1][j-1]=='X'){
          neighbors++;
        }else if(block[i-1][j]=='X'){
          neighbors++;
        }else if(block[i-1][j+1]=='X'){
          neighbors++;
        }else if(block[i][j-1]=='X'){
          neighbors++;
        }else if(block[i][j+1]=='X'){
          neighbors++;
        }else if(block[i+1][j-1]=='X'){
          neighbors++;
        }else if(block[i+1][j]=='X'){
          neighbors++;
        }else if(block[i+1][j+1]=='X'){
          neighbors++;
        }
        changed=0;
        if(block[i][j]=='X'){
          changed=1;
          if(neighbors<=1){
            change[i][j]='D';
          }else if(neighbors==2 || neighbors==3){
            change[i][j]='L';
          }else if(neighbors>3){
            change[i][j]='D';
          }
        }else{
          if(neighbors==3){
            changed=1;
            change[i][j]='L';
          }
        }
        if(changed==0)change[i][j]='N';
    }
  }
  for(i=1;i<blockDimension-1;i++){
    for(j=1;j<blockDimension-1;j++){
      if(change[i][j]=='D'){
        block[i][j]='O';
      }else if(change[i][j]=='L'){
        block[i][j]='X';
      }
    }
  }

  for(i=0;i<blockDimension;i++){
    free(block[i]);
  }
  free(block);
  fclose(fp);
  MPI_Finalize();
  return 0;
}
