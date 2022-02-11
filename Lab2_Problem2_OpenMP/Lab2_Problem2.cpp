/*
Description:
    Solution file for Problem 2 in Lab 2
    Implemented Numerical integration using mid-point rule
       and openmp library for multithreading
*/

#include <iostream>
#include <cmath>
#include <omp.h>
#include <string>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <iomanip>

/*
* Function to check whether the input argument is a number,
* if so, then set the number through reference
* else, return false
*
* @param charsToCheck char array to verify
* @param ulInNumber reference to original variable to set
* Returns: bool if input is a number
*/
bool convertToNumbers(const char* charsToCheck, double& ulInNumber)
{
    try
    {
        // Check to see if the char array is a number
        // Using lambda function to check if each char is a number
        bool check = std::all_of(charsToCheck, charsToCheck + strlen(charsToCheck),
            [](unsigned char c) { return ::isdigit(c); });
        if (check)
        {
            // Char array is a valid number 
            std::string strToCheck(charsToCheck);
            ulInNumber = std::stod(strToCheck);
            if (ulInNumber >= 0)
            {
                return true;
            }
        }
        return false;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

int main(int argc, char* argv[])
{
    // Open output stream to write data into out file
    std::ofstream ofOutFile;
    ofOutFile.open("Lab2Prob2.txt", std::ios::trunc);

    if (not ofOutFile.is_open())
    {
        // If unable to open output file => print error
        std::cerr << "Unable to open output file: Lab2Prob2.txt" << std::endl;
        return 1;
    }

    if (argc != 2)
    {
        // Check 1: Expected input args = 1 (+ the executable)
        // Print error if more or less inputs provided
        ofOutFile << "Invalid inputs";
        ofOutFile.close();
        return 1;
    }

    double stepNumber{ 0 };

    // Convert the char array into the required number
    if (not convertToNumbers(argv[1], stepNumber))
    {
        // Check 2: Print error if the input is not a number
        ofOutFile << "Invalid inputs";
        ofOutFile.close();
        return 1;
    }

    // store the lower and upper bound
    double lowerBound = 0;
    double upperBound = log(2) / 7;

    // store the step size
    double stepSize = (upperBound - lowerBound) / stepNumber;

    // tracker for total sum
    double sum = 0;

    int maxThreadCount = omp_get_max_threads(); // storing to avoid calling again in each thread

    // Range of each thread
    double threadRange = (upperBound - lowerBound) / maxThreadCount;

    // init variables for thread
    double partialSum{ 0 }, threadLowerBound{ 0 }, threadUpperBound{ 0 };
    int threadNum{ 0 };

#pragma omp parallel private (threadNum, partialSum, threadLowerBound, threadUpperBound)
    {
        // Get thread number
        threadNum = omp_get_thread_num();
        // Get lower bound for summation
        threadLowerBound = lowerBound + threadRange * threadNum;
        // Get upper bound for summation
        threadUpperBound = threadLowerBound + threadRange;

        // Iterate through the bounds and add to partial sum
        for (; threadLowerBound <= threadUpperBound; threadLowerBound += stepSize)
        {
            partialSum += 14 * exp(7 * threadLowerBound) * stepSize;
        }

        // Add the partial sum to the main sum
#pragma omp critical
        // Critical section to protect the shared variable sum during write
        sum += partialSum;
    }

    // Print output to the file and close
    ofOutFile << std::fixed << std::setprecision(6) << sum;
    ofOutFile.close();
    return 0;
}
