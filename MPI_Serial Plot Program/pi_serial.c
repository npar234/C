
#include<stdio.h>
#include<stdlib.h>
#include<time.h>

int main(int argc, char* argv[]){	
	/* checks for correct command line format */
	if(argc != 2){
		printf("Usage: ./progname [number_points]\n");
		exit(0);
	}
	
	/* gets number of points from command line */
	int numPoints = atoi(argv[1]);
	
	/* prints error if negative number of points given */
	if(numPoints < 1){
		printf("Please enter a positive number of points\n");
		exit(0);
	}


	double circle;
	double square;
	/* all points are in the square */
	square = numPoints;
	
	/* radius of circle */
	const double r = 0.5;
	const double r2 = 0.5*0.5;
	
	/* seeds the random number generator */
	srand(time(NULL));
	
	int i;
	for(i = 0; i < numPoints; i++){
		/* generate random number between 0 and 1 and then subract 0.5 */
		double x = (((double)rand())/ ((double)RAND_MAX))-r;
		double y = (((double)rand())/ ((double)RAND_MAX))-r;
		
		if(((x*x)+(y*y))<= r2){
			circle++;
		}
	}
	
	/* calculates and prints final value */
	double finalProb = circle / square;
	double pi = finalProb*4;

	printf("Estimated value of pi: %0.15f\n", pi);

	return 0;
}
