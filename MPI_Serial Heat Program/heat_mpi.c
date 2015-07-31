
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define WHITE    "15 15 15 "
#define RED      "15 00 00 "
#define ORANGE   "15 05 00 "
#define YELLOW   "15 10 00 "
#define LTGREEN  "00 13 00 "
#define GREEN    "05 10 00 "
#define LTBLUE   "00 05 10 "
#define BLUE     "00 00 10 "
#define DARKTEAL "00 05 05 "
#define BROWN    "03 03 00 "
#define BLACK    "00 00 00 "

float** InitNew0(int dime, int size, int *x0);
float** InitNew(int dime, int size, int *x, int rank);
float** InitNew1(int dime, int size, int *x1);
void CopyNewToOld(float*** new, float*** old, int x, int y);
void CalculateNew(float*** new, float*** old, int x, int y);
void PrintGrid(float*** grid, int x, int y);
void PrintGUI(float*** grid, int dime);;

int main(int argc, char* argv[]){
  if(argc != 3){
    printf("Usage: ./progname [iteration] [dimension]\n");
    exit(0);
  } 
  int iter = atoi(argv[1]);
  int dime = atoi(argv[2]);
  if( (iter <= 0) || (dime <= 0) ){
    printf("Please enter values greater than 0\n");
    exit(0);
  }
  
  int rank, size;
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
 
  int i, j;
  int x0, x, x1;

  /* "new" and "old" segment arrays to store / split up work */
  float **new = NULL;
  float **old = NULL;
  if(rank == 0){
    new = InitNew0(dime, size, &x0);
    old = InitNew0(dime, size, &x0);
  }
  else if(rank < (size - 1)){
    new = InitNew(dime, size, &x, rank);
    old = InitNew(dime, size, &x, rank);
  }
  else{
    new = InitNew1(dime, size, &x1);
    old = InitNew1(dime, size, &x1);
  }

  /* runs through all iterations */
  for(i = 0; i < iter; i++){
    /* first part updates ghost rows every iteration */
    if(rank == 0){
      MPI_Send(new[x0-2], dime, MPI_FLOAT, (rank+1), 0, MPI_COMM_WORLD);
      MPI_Recv(new[x0-1], dime, MPI_FLOAT, (rank+1), 0, MPI_COMM_WORLD, &status);
    }
    else if(rank < (size-1)){
      MPI_Send(new[1], dime, MPI_FLOAT, (rank-1), 0, MPI_COMM_WORLD);
      MPI_Recv(new[0], dime, MPI_FLOAT, (rank-1), 0, MPI_COMM_WORLD, &status);
      MPI_Send(new[x-2], dime, MPI_FLOAT, (rank+1), 0, MPI_COMM_WORLD);
      MPI_Recv(new[x-1], dime, MPI_FLOAT, (rank+1), 0, MPI_COMM_WORLD, &status);
    }
    else{
      MPI_Send(new[1], dime, MPI_FLOAT, (rank-1), 0, MPI_COMM_WORLD);
      MPI_Recv(new[0], dime, MPI_FLOAT, (rank-1), 0, MPI_COMM_WORLD, &status);
    }
    MPI_Barrier(MPI_COMM_WORLD);
  
    /* once ghost rows are updated, program proceeds to 
        copy new to old and calculate new values */
    if(rank == 0){
      CopyNewToOld(&new, &old, x0, dime);
      CalculateNew(&new, &old, x0, dime);
    }
    else if(rank < (size-1)){
      CopyNewToOld(&new, &old, x, dime);
      CalculateNew(&new, &old, x, dime);      
    }
    else{
      CopyNewToOld(&new, &old, x1, dime);
      CalculateNew(&new, &old, x1, dime);
    }
  }
  
  float **grid = NULL;
  if(rank == 0){
    /* initialize grid array */
    grid = malloc(dime * sizeof(float*));
    for(i = 0; i < dime; i++){
      grid[i] = malloc(dime * sizeof(float));
    }
  
  /* once all iterations are completed results are combined
      into the grid array */  
    for(i = 0; i < (x0-1); i++){
      for(j = 0; j < dime; j++){
        grid[i][j] = new[i][j];
      }
    }
    int counter = (x0-1);
    for(i = 1; i < size; i++){
      for(j = 0; j < (dime/size); j++){
        MPI_Recv(grid[counter], dime, MPI_FLOAT, i, 0, MPI_COMM_WORLD, &status);
        counter++;
      }
    }
  }
  else if(rank < (size-1)){
    for(i = 1; i < (x-1); i++){
      MPI_Send(new[i], dime, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }
  }
  else{
    for(i = 1; i < x1; i++){
      MPI_Send(new[i], dime, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }
  }
  
  /* print result to image file */
  if(rank == 0){
    /* PrintGrid(&grid, dime, dime); */
    PrintGUI(&grid, dime);
  }
  
  /* finish up program */
  MPI_Finalize();
  return 0;
}

/* initializes a new array for top most part of grid */
float** InitNew0(int dime, int size, int *x0){
  int i,j;
  int tx = (dime / size) + (dime % size) + 1;
  float** anew = malloc(tx * sizeof(float*));
  for(i = 0; i < dime; i++){
    anew[i] = malloc(dime * sizeof(float));
  }
  (*x0) = tx;
  
  for(i = 0; i < (*x0); i++){
    for(j = 0; j < dime; j++){
      anew[i][j] = 20;
    }
  }
  
  int lower = dime*.30;
  int upper = dime*.70;
  for(i = lower; i <= upper; i++){
    anew[0][i] = 300;
  }
  return anew;
}

/* initializes array for middle portion of grid */
float** InitNew(int dime, int size, int *x, int rank){
  int i,j;
  int tx = (dime/size) + 2;
  float** anew = malloc(tx * sizeof(float*));
  for(i = 0; i < dime; i++){
    anew[i] = malloc(dime * sizeof(float));
  }
  (*x) = tx;
  
  for(i = 0; i < (*x); i++){
    for(j = 0; j < dime; j++){
      anew[i][j] = 20;
    }
  }
  return anew;
}

/* initializes array for bottom portion of grid */
float** InitNew1(int dime, int size, int *x1){
  int i,j;
  int tx = (dime / size) + 1;
  float** anew = malloc(tx * sizeof(float*));
  for(i = 0; i < dime; i++){
    anew[i] = malloc(dime * sizeof(float));
  }
  (*x1) = tx;

  for(i = 0; i < (*x1); i++){
    for(j = 0; j < dime; j++){
      anew[i][j] = 20;
    }
  }  
  return anew;
}

void CopyNewToOld(float*** new, float*** old, int x, int y){
  int i,j;  
  for(i = 0; i < x; i++){
    for(j = 0; j < y; j++){
      (*old)[i][j] = (*new)[i][j];
    }
  }
}

void CalculateNew(float*** new, float*** old, int x, int y){
  int i,j;
  for(i = 1; i < (x-1); i++){
    for(j = 1; j < (y-1); j++){
      (*new)[i][j] = 0.25*((*old)[i-1][j]+(*old)[i+1][j]+(*old)[i][j-1]+(*old)[i][j+1]);
    }
  }
}


void PrintGrid(float*** grid, int x, int y){
  int i,j;
  for(i = 0; i < x; i++){
    for(j = 0; j < y; j++){
      printf("%.2f\t", (*grid)[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

void PrintGUI(float*** grid, int dime){
  int i, j;
  FILE *fp;
  fp = fopen("heat_serial.pnm", "w");
  fprintf(fp, "P3\n%d %d\n15\n", dime, dime);

  float temp;
  for(i = 0; i < dime; i++){
    for(j = 0; j < dime; j++){
      temp = (*grid)[i][j];
      if( temp > 250){
        fprintf(fp, "%s ", RED);
      }
      else if( temp > 180 && temp <=250){
        fprintf(fp, "%s ", ORANGE);
      }
      else if( temp > 120 && temp <=180){
        fprintf(fp, "%s ", YELLOW);
      }
      else if( temp > 80 && temp <=120){
        fprintf(fp, "%s ", LTGREEN);
      }
      else if( temp > 60 && temp <=80){
        fprintf(fp, "%s ", GREEN);
      }  
      else if( temp > 50 && temp <=60){
        fprintf(fp, "%s ", LTBLUE);
      }  
      else if( temp > 40 && temp <=50){
        fprintf(fp, "%s ", BLUE);
      }  
      else if( temp > 30 && temp <=40){
        fprintf(fp, "%s ", DARKTEAL);
      }  
      else if( temp > 20 && temp <=30){
        fprintf(fp, "%s ", BROWN);
      }     
      else{
        fprintf(fp, "%s ", BLACK);
      };
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
  system("convert heat_serial.pnm heat_serial.gif");
}
