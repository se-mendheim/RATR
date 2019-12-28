/**************************************************************************
* Samuel Mendheim - COMP 233 - Ring Around The Rosie
*
* This program is to test the time it takes for processes to pass an array
*  filled with a message around to each other. The master will calculate
*  the time it takes to come back around to itself and calculate the min, 
*  max and average of these times.
*
**************************************************************************/

#include <mpi.h>
#include <stdio.h>



int main(int argC, char* argv[]) {

	// initialize the parallel block
	MPI_Init(&argC, &argv);

	// time variables to get the current time and calculate total time
	double startTime, finishTime, totalTime;
	// time calculation variables
	double  minTime = 0, maxTime = 0, avgTime = 0, avgTimeSum = 0;
	// variables to store the world size and the world rank
	int world_size;
	int world_rank;
	// for loop variables for controlling the processes
	int numTrial, r, diffSize, checkArray;
	// the size of the array which will change on iterations
	int sizeOfArray = 0;


	// get the comm size of all the parallel processes
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	// get the comm rank of each individual process
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	if (world_rank == 0) {
		// used for printing a pretty table
		printf("\n    Size Array(B) \tMax Time(S)\t\tMin Time(S)\t\tAvg Time(S)\n\n");
		printf("---------------------------------------------"
			"-------------------------------------------\n");
	}


	// loop to control different array sizes
	for (diffSize = 0; diffSize < 12; diffSize++) {
		// if it is the first iteration
		if (diffSize == 0) {
			// setting the size of the array to 1,000
			sizeOfArray = 1000;
		}
		// if it is the second iteration
		else if (diffSize == 1) {
			// setting the size of the array to 50,000
			sizeOfArray = 50000;
		}
		else  if (diffSize == 2) {
			// adding the previous amount from 50,000 to achieve a final amount of 1,000,000
			sizeOfArray = 100000;
		}
		else {
			// continually incremeneting the array size by 100,000
			sizeOfArray += 100000;
		}

		// allocate memory for the dynamic array
		double* msg = (double*)malloc(sizeOfArray * sizeof(double));

		// number of trials to calculate time
		for (numTrial = 0; numTrial < 10; numTrial++) {

			// if the master is the current process
			if (world_rank == 0) {

				// fill the array
				for (r = 0; r < sizeOfArray; r++) {
					// adding elements with 
					msg[r] = (double)world_rank + sizeOfArray;
				}
				// get the start time before the message sends
				startTime = MPI_Wtime();
				// send the message to the first process
				MPI_Send(msg, sizeOfArray, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
				// recieve the message from the last process
				MPI_Recv(msg, sizeOfArray, MPI_DOUBLE, world_size - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				// stop timing
				finishTime = MPI_Wtime();

				// for loop to check if the message came through correct
				for (checkArray = 0; checkArray < diffSize; checkArray++) {
					// if the message came through incorrectly return -1;
					if (msg[checkArray] != ((double)world_rank + sizeOfArray)) {
						printf("Message came through incorrectly");
						return -1;
					}
				}
			}
			else {
				// recieve the message from the previous process
				MPI_Recv(msg, sizeOfArray, MPI_DOUBLE, world_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				// send it to the next process or back around to master
				MPI_Send(msg, sizeOfArray, MPI_DOUBLE, (world_rank + 1) % world_size, 0, MPI_COMM_WORLD);
			}

			// record the time of the entire process
			if (world_rank == 0) {
				// calculate time taken to send the message
				totalTime = finishTime - startTime;
				// get the total count of all of the trials
				avgTimeSum += totalTime;
				// calculate the average time using the sum total of times
				avgTime = avgTimeSum / (numTrial + 1);

				// if the first iteration of the trials
				if (numTrial == 0) {
					// set the minimum time equal to the first time recorded
					minTime = totalTime;
				}
				// if the time recorded is greater than the current maximum
				if (totalTime > maxTime) {
					// set the maximum to the time recorded
					maxTime = totalTime;
				}
				// if the time recorded is less than the current minimum
				else if (totalTime < minTime) {
					// set the minimum to the time recorded
					minTime = totalTime;
				}
			}
		}

		if (world_rank == 0) {
			// printing out the results to the table
			printf("\t%d\t | \t%f\t | \t%f\t | \t%f\n", sizeOfArray, maxTime, minTime, avgTime);
			printf("------------------------------------------"
				"----------------------------------------------\n");
		}
		// reset variables for calculating time
		maxTime = minTime = avgTimeSum = 0;

		// free up the allocated memory
		free(msg);

	}

	if (world_rank == 0) {
		printf("\n\n< Normal Termination >\n\n");
	}

	// finish up the parallel block
	MPI_Finalize();

	return 0;
}
