//
// Starting code for the portfolio exercise. Some required routines are included in a separate
// file (ending '_extra.h'); this file should not be altered, as it will be replaced with a different
// version for assessment.
//
// Compile as normal, e.g.,
//
// > gcc -o portfolioExercise portfolioExercise.c
//
// and launch with the problem size N and number of threads p as command line arguments, e.g.,
//
// > ./portfolioExercise 12 4
//


//
// Includes.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "portfolioExercise_extra.h"        // Contains routines not essential to the assessment.

// thread struct
typedef struct {
    int threadID;
    int start;
    int end;
    int N;
    float **M; // matrix pointer for multuiplicatyion
    float *u; //input vector
    float *v; //output vector
    float *partialSums; // array to store partial dot products
} ThreadData;

// worker function
void* worker(void* arg)
{ // pthreads needs to return void* and void* argument
    ThreadData* data = (ThreadData*)arg;

    float localSum = 0.0f; // store threads partial dot product

    // matrix vector multiplication 
    for (int row = data->start; row < data->end; row++) { // split  into chunks
        data->v[row] = 0.0f; // initalise output for this chunk
        for (int col = 0; col < data->N; col++) {
            data->v[row] += data->M[row][col] * data->u[col];
        }
    }

    // partial dot producct
    for (int i = data->start; i < data->end; i++) {
        localSum += data->v[i] * data->v[i];
    }

    data->partialSums[data->threadID] = localSum; // store threads partial dot propduct in shared array

    return NULL; // show thread has finished
}

//
// Main.
//
int main( int argc, char **argv )
{
    //
    // Initialisation and set-up.
    //

    // Get problem size and number of threads from command line arguments.
    int N, nThreads;
    if( parseCmdLineArgs(argc,argv,&N,&nThreads)==-1 ) return EXIT_FAILURE;

    // Initialise (i.e, allocate memory and assign values to) the matrix and the vectors.
    float **M, *u, *v;
    if( initialiseMatrixAndVector(N,&M,&u,&v)==-1 ) return EXIT_FAILURE;

    // For debugging purposes; only display small problems (e.g., N=8 and nThreads=2 or 4).
    if( N<=12 ) displayProblem( N, M, u, v );

    // Start the timing now.
    struct timespec startTime, endTime;
    clock_gettime( CLOCK_REALTIME, &startTime );

    //
    // Parallel operations, timed.
    //
    float dotProduct = 0.0f;        // You should leave the result of your calculation in this variable.

    // Step 1. Matrix-vector multiplication Mu = v.
    // Create thread arrays
    pthread_t threads[nThreads]; // array to hold thread ids
    ThreadData threadData[nThreads]; // array of structs holding each threads info
    float partialSums[nThreads]; // store each threads specific dot product

    int chunk = N / nThreads;

    // Create threads
    // loop over threads, initialising their data
    for (int thread = 0; thread < nThreads; thread++) {
        threadData[thread].threadID = thread;
        // decide which part of the sum this thread will complete
        threadData[thread].start = thread * chunk;
        threadData[thread].end = (thread + 1) * chunk;
        threadData[thread].N = N;
        threadData[thread].M = M;
        threadData[thread].u = u;
        threadData[thread].v = v;
        threadData[thread].partialSums = partialSums;

        pthread_create(&threads[thread], NULL, worker, &threadData[thread]); // make new thread, call worker function, pass pointer to threads data struct
    }

    // Join threads
    // Wait for all threads to finish before continuing
    for (int thread = 0; thread < nThreads; thread++) {
        pthread_join(threads[thread], NULL);
    }


    // After completing Step 1, you can uncomment the following line to display M, u and v, to check your solution so far.
    // if( N<=12 ) displayProblem( N, M, u, v );

    // Step 2. The dot product of the vector v with itself.
    // combine all the partial sums
    for (int thread = 0; thread < nThreads; thread++) {
        dotProduct += partialSums[thread];
    }

    // DO NOT REMOVE OR MODIFY THIS PRINT STATEMENT AS IT IS REQUIRED BY THE ASSESSMENT.
    printf( "Result of parallel calculation: %f\n", dotProduct );

    //
    // Check against the serial calculation.
    //

    // Output final time taken.
    clock_gettime( CLOCK_REALTIME, &endTime );
    double seconds = (double)( endTime.tv_sec + 1e-9*endTime.tv_nsec - startTime.tv_sec - 1e-9*startTime.tv_nsec );
    printf( "Time for parallel calculations: %g secs.\n", seconds );

    // Step 1. Matrix-vector multiplication Mu = v.
    for( int row=0; row<N; row++ )
    {
        v[row] = 0.0f;              // Make sure the right-hand side vector is initially zero.

        for( int col=0; col<N; col++ )
            v[row] += M[row][col] * u[col];
    }

    // Step 2: The dot product of the vector v with itself
    float dotProduct_serial = 0.0f;
    for( int i=0; i<N; i++ ) dotProduct_serial += v[i]*v[i];

    // DO NOT REMOVE OR MODIFY THIS PRINT STATEMENT AS IT IS REQUIRED BY THE ASSESSMENT.
    printf( "Result of the serial calculation: %f\n", dotProduct_serial );

    //
    // Clear up and quit.
    //
    freeMatrixAndVector( N, M, u, v );

    return EXIT_SUCCESS;
}