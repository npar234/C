
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<mpi.h>
#include<math.h>

int main(int argc, char* argv[]){
	if(argc != 2){
		printf("Usage: mpirun -np numProcesses ./progname [numPoints]\n", argv[0]);
		exit(0);
	}
	
	int numPoints = atoi(argv[1]);
	
	if(numPoints < 1){
		printf("Please enter a positive number of points\n");
		exit(0);
	}
	
	int rank, size;
	double startTime, endTime;
	double compute_time, compute_start, compute_end;
	double x1, x2, y1, y2;
	double r = 0.5;
	double r2 = 0.5*0.5;
	
	/* initializes MPI and get rank and size into the rank and size variables */
	MPI_Init(&argc, &argv);
	/* starting time of mpi */
	
	/********************************************************
	start program total time
	*********************************************************/
	startTime = MPI_Wtime();
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Status status;
	
	/***********************************************************
	Computations
	***********************************************************/
	/* rank 0 takes care of the "extra" points if an odd number of points
		are give. For example 999 points with 4 processes, 0 takes the
		remaining points
	*/
	
	/*****************************
	start computations time
	*****************************/
	compute_start = MPI_Wtime();
	double myPoints;
	if(rank == 0){
		myPoints = (numPoints / size) + (numPoints % size);
	}
	else{
		myPoints = numPoints / size;
	}
	
	/* calculates individual section of square based on given formula */
	x1 = (1/sqrt(size)) * (rank % (int)sqrt(size));
	x2 = x1 + (1/sqrt(size));
	
	y1 = (1/sqrt(size)) * floor(rank / sqrt(size));
	y2 = y1 + (1/sqrt(size));
	
	/* seeds the random number generator */
	srand(time(NULL));
	double circle = 0;
	double diffx = x2 - x1;
	double diffy = y2 - y1;
	int i; 
	for(i = 0; i < myPoints; i++){
		/* calculates random points for x and y */
		double randx = ((double)rand())/((double)RAND_MAX);
		double randy = ((double)rand())/((double)RAND_MAX);
		
		/* uses formula to get value in processes square sector. Then subtacts
			0.5 to get a value within limits */
		double x = ((diffx * randx) + x1) - r;
		double y = ((diffy * randy) + y1) - r;
		
		/* adds to circle if point is within the circle */
		if(((x*x) + (y*y)) <= r2){
			circle++;
		}
	}
	/*****************************
	end computation time
	******************************/	
	compute_end = MPI_Wtime();
	compute_time = compute_end - compute_start;
	
	
	/**********************************************************
	Communications
	**********************************************************/
	if(rank == 0){
		printf("\n");
		printf("Size (num processes): %d\n", size);
		
		/* keeps track of total number of circle points */
		double totalCircle = 0;
		totalCircle += circle;
		
		/* total time for rank 0 */
		double total_time = 0;
		total_time += compute_time;
		
		double temp = 0;
		
		/***************
		start communication time 
		*****************/
		double comm_start = MPI_Wtime();
		/* receives number of circle points from other processes */
		for(i = 1; i < size; i++){
			MPI_Recv(&temp, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD, &status);
			totalCircle += temp;
		}
		/*****************
		end partial communication time
		*******************/
		double comm_end = MPI_Wtime();
		
		double tempt = 0;
		for(i = 1; i < size; i++){
			MPI_Recv(&tempt, 1, MPI_DOUBLE, i, 2, MPI_COMM_WORLD, &status);
			total_time += tempt;
		}
		
		/******************
		end full communication time
		******************/
		double comm_end_full = MPI_Wtime();
		
		/* calculates estimated pi value and total execution time */
		double finalProb = totalCircle / numPoints;
		double pi = finalProb * 4;
		
		double compute_time_avg = total_time / size;
		double comm_time = comm_end - comm_start;
		double comm_time_full = comm_end_full - comm_start;
		
		/************************************************************
		end program total time
		*********************************************************/
		endTime = MPI_Wtime();	
		
		double totalTime = endTime - startTime;	

		printf("Estimated value of pi: %0.15f\n", pi);
		printf("Total Computation (Monte Carlo Simulation) time: %.10f\n", compute_time_avg);
		printf("Total Communication time (not including compute_time transfers): %0.10f\n", comm_time);
		printf("Total Communication time (including compute_time transfers): %0.10f\n", comm_time_full);
		printf("Time taken to run program %.10f\n", totalTime);
		printf("\n");
	}
	else{
		/* sends number of circle points to root process */
		MPI_Send(&circle, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
		MPI_Send(&compute_time, 1, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
	}
	
	MPI_Finalize();
	return 0;
}
