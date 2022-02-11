/*
Description:
    Solution file for Lab 5.
    Implemented the solution to the heat plate problem using CUDA.
    1. The code initializes an input array for the initial temperature of the hot plate.
    2. It uses the averaging method to find the temperature of each element based on the
    temperature of its neighbours.
    3. Then it uses the jacobi iteration to assign the new temperature values back to the
    hot plate array.

    Top level reference for code: (as provided by Instructors on Piazza in Lab 5 Meta Thread @797, Author: Kuo-Wei-Lai)
    Piazza Link: https://piazza.com/class/ks93yix32l4s0?cid=797
    Code Reference: https://github.com/NVIDIA/cuda-samples/blob/master/Samples/vectorAdd/vectorAdd.cu
*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <cuda_runtime.h>

/*
* Function to check if command line option provided
* @param argc -> number of command line args
* @param argv -> array of command line args chars
* @param inputToCheck -> input argument to find
* @return int -> index of command line argument
*/
int checkCommandLineOption(int argc, char** argv, const char inputToCheck[])
{
    for (int i = 1; i < argc; ++i)
    {
        // compare strings
        if (strlen(argv[i]) != strlen(inputToCheck))
        {
            continue;
        }
        bool isAllEqual = true;
        for (int j = 0; j < strlen(inputToCheck); ++j)
        {
            if (argv[i][j] != inputToCheck[j])
            {
                isAllEqual = false;
                break;
            }
        }
        if (isAllEqual == true)
        {
            return i;
        }
    }
    return -1;
}

/*
* Function to print the error message for invalid command line inputs
*/
void printCommandLineError()
{
    printf("Invalid parameter, please check your values.\n");
}

/*
* Function to check for errors returns by the CUDA APIs
* @param errCode -> error code
* @param line -> line in which the error occured
*
* Reference for below code: helper_cuda.h in cuda-samples-master.zip provided by Instructors in Canvas
*/
void checkCudaError(cudaError_t code, int line)
{
    if (code != cudaSuccess)
    {
        fprintf(stderr, "checkCudaError: %s %d\n", cudaGetErrorString(code), line);
        exit(code);
    }
}

/*
* CUDA Kernel Device code
*
* Computes the new temperature of each index in the row-column matrix using
* the averaging method by taking the average of its 4 neighbours.
* @param outTemp -> pointer to the output temperature array
* @param inTemp -> pointer to the input temperature array
* @param dataSize -> size of each row of data
*/
__global__ void vector_update_temp(double* outTemp, const double* inTemp, const int dataSize)
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    int row = tid / dataSize;
    int col = tid % dataSize;
    // check if current element to check is either of th edge
    if (row >= dataSize || col >= dataSize)
    {
        return;
    }
    if (row == 0 || row == dataSize - 1 || col == 0 || col == dataSize - 1)
    {
        // copy forward the same temperature
        outTemp[tid] = inTemp[tid];
    }
    else
    {
        // if tid is not on any edge, then do below
        int top = (row - 1) * dataSize + col;
        int bottom = (row + 1) * dataSize + col;
        int left = row * dataSize + col - 1;
        int right = row * dataSize + col + 1;

        outTemp[tid] = (inTemp[top] + inTemp[bottom] + inTemp[left] + inTemp[right]) / 4;
    }
}

/*
* Main function to be run on the host machine
*/
int main(int argc, char** argv)
{
    int numInnerPoints = 0, numIter = 0, argIndex = 0;
    argIndex = checkCommandLineOption(argc, argv, "-q");
    if (argIndex != -1)
    {
        return 0;
    }
    argIndex = checkCommandLineOption(argc, argv, "-N");
    if (argIndex != -1)
    {
        numInnerPoints = atof(argv[argIndex + 1]);
        if (numInnerPoints == 0)
        {
            printCommandLineError();
            return 1;
        }
    }
    else
    {
        printCommandLineError();
        return 1;
    }
    argIndex = checkCommandLineOption(argc, argv, "-I");
    if (argIndex != -1)
    {
        numIter = atof(argv[argIndex + 1]);
        if (numInnerPoints == 0)
        {
            printCommandLineError();
            return 1;
        }
    }
    else
    {
        printCommandLineError();
        return 1;
    }

    // Variable to hold the dataSize based on number of inner points (+2 edges)
    int dataSize = numInnerPoints + 2;
    // Variable to hold the size to allocate memory for all arrays required
    size_t size = (dataSize * dataSize) * sizeof(double);

    // Pointers for the host machine temperature arrays, in and out
    double* h_inTemp = (double*)malloc(size);
    double* h_outTemp = (double*)malloc(size);

    // Verify if the array init was successful
    if (h_inTemp == NULL || h_outTemp == NULL)
    {
        fprintf(stderr, "Unable to allocate host array.\n");
        exit(EXIT_FAILURE);
    }

    // Init temp values
    for (int i = 0; i < dataSize; ++i)
    {
        for (int j = 0; j < dataSize; ++j)
        {
            int dataIndex = i * dataSize + j;
            h_inTemp[dataIndex] = 20.0;
            // check if it is in the 40 % of top row
            if (i == 0 && j > 0.3 * (dataSize - 1) && j < 0.7 * (dataSize - 1))
            {
                h_inTemp[dataIndex] = 100.0;
            }
        }
    }

    // Init the data on the device for input temp
    double* d_inTemp = NULL;
    checkCudaError(cudaMalloc((void**)&d_inTemp, size), __LINE__);

    // Init the data on the device for output temp
    double* d_outTemp = NULL;
    checkCudaError(cudaMalloc((void**)&d_outTemp, size), __LINE__);

    // Copy the host input temp to the device input temp
    checkCudaError(cudaMemcpy(d_inTemp, h_inTemp, size, cudaMemcpyHostToDevice), __LINE__);

    // Calculate the threads and blocks needed to launch CUDA kernel
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, 0);
    int threadsPerBlock = prop.maxThreadsPerBlock;
    int blocksPerGrid = (dataSize * dataSize + threadsPerBlock - 1) / threadsPerBlock;

    // Creating cuda events to use to time profiling
    // References from matrixMul.cu example in cuda-samples-master.zip
    // provided by Instructors in the modules section of Canvas
    cudaEvent_t startEvent, stopEvent;
    checkCudaError(cudaEventCreate(&startEvent), __LINE__);
    checkCudaError(cudaEventCreate(&stopEvent), __LINE__);

    // Record the start event before starting kernel operations
    checkCudaError(cudaEventRecord(startEvent, NULL), __LINE__);

    // Iterate over the temperature resolution logic for the required number of times
    for (int i = 0; i < numIter; ++i)
    {
        // Run the temperature update function on the kernel
        vector_update_temp << <blocksPerGrid, threadsPerBlock >> > (d_outTemp, d_inTemp, dataSize);
        // Check for erros
        checkCudaError(cudaGetLastError(), __LINE__);
        // Copy the in output temperature into the input temperature for jacobi iteration
        checkCudaError(cudaMemcpy(d_inTemp, d_outTemp, size, cudaMemcpyDeviceToDevice), __LINE__);
    }

    // Record the stop event
    checkCudaError(cudaEventRecord(stopEvent, NULL), __LINE__);

    // Wait for the stop event to complete
    checkCudaError(cudaEventSynchronize(stopEvent), __LINE__);

    float timeElapsed = 0.0f;
    checkCudaError(cudaEventElapsedTime(&timeElapsed, startEvent, stopEvent), __LINE__);

    // Copy the device output temp data into the host output temp
    checkCudaError(cudaMemcpy(h_outTemp, d_outTemp, size, cudaMemcpyDeviceToHost), __LINE__);


    // Write data into file
    FILE* filePointer;
    filePointer = fopen("finalTemperatures.csv", "w");

    // Print output temp
    for (int i = 0; i < dataSize; ++i)
    {
        for (int j = 0; j < dataSize; ++j)
        {
            fprintf(filePointer, "%0.6lf,", h_outTemp[i * (int)dataSize + j]);
        }
        fprintf(filePointer, "\n");
    }
    fclose(filePointer);

    // Free the device data arrays
    checkCudaError(cudaFree(d_inTemp), __LINE__);
    checkCudaError(cudaFree(d_outTemp), __LINE__);

    // Free the host data arrays
    free(h_inTemp);
    free(h_outTemp);

    // Print the time elapsed and exit
    printf("%0.2f\n", timeElapsed);

    return 0;
}
