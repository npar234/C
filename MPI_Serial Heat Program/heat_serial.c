
#include <stdio.h>
#include <stdlib.h>

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

void CopyNewToOld(float*** new, float*** old, int dime);
void CalculateNew(float*** new, float*** old, int dime);
void PrintGrid(float*** grid, int dime);
void PrintGUI(float*** grid, int dime);

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
  
  /* initialize grid for temperatures */
  int i, j;
  float **new = malloc(dime * sizeof(float*));
  for(i = 0; i < dime; i++){
    new[i] = malloc(dime * sizeof(float));
  }
  
  /* set temperatures to 20 */
  for(i = 0; i < dime; i++){
    for(j = 0; j < dime; j++){
      new[i][j] = 20;
    }
  }
  
  /* set fireplace to 300 */
  int lower = dime*.30;
  int upper = dime*.70;
  for(i = lower; i <= upper; i++){
    new[i][0] = 300;
  }

  /* generate grid for "old" data */  
  float **old = malloc(dime * sizeof(float*));
  for(i = 0; i < dime; i++){
    old[i] = malloc(dime * sizeof(float));
  }
  
  /* runs through iterations */
  for(i = 0; i < iter; i++){
    CopyNewToOld(&new, &old, dime);
    CalculateNew(&new, &old, dime);
    /* PrintGrid(&new, dime); */
  }
  
  PrintGUI(&new, dime);
  
  /* free memory */
  for(i = 0; i < dime; i++){
    free(new[i]);
    free(old[i]);
  }
  free(new);
  free(old);

  return 0;
}

void CopyNewToOld(float*** new, float*** old, int dime){
  int i,j;  
  for(i = 0; i < dime; i++){
    for(j = 0; j < dime; j++){
      (*old)[i][j] = (*new)[i][j];
    }
  }
}

void CalculateNew(float*** new, float*** old, int dime){
  int i,j;
  for(i = 1; i < (dime-1); i++){
    for(j = 1; j < (dime-1); j++){
      (*new)[i][j] = 0.25*((*old)[i-1][j]+(*old)[i+1][j]+(*old)[i][j-1]+(*old)[i][j+1]);
    }
  }
}

void PrintGrid(float*** grid, int dime){
  int i,j;
  for(i = 0; i < dime; i++){
    for(j = 0; j < dime; j++){
      printf("%.2f\t", (*grid)[j][i]);
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
      temp = (*grid)[j][i];
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
      }
    }
    fprintf(fp, "\n");
  }
  
  fclose(fp);
  
  system("convert heat_serial.pnm heat_serial.gif");
}
