/*
Description:
    Solution file for Problem 1 in Lab 2.
    Implemented the ant and seeds problem using standard threading library
*/

#include <iostream>
#include <random>
#include <utility>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cmath>
#include <array>
#include <fstream>
#include <iomanip>

std::mutex mtxStatsWrite;
long double totalRuns = 0;
long double totalSteps = 0;

/*
* Function to find steps taken for ant to move all seeds
* from the bottom row to the top row
* 
* @param numRuns -> numbers of times the whole problem should be run
*/
void antTravelGird(const unsigned long numRuns)
{
    // creating random number generator per thread
    std::random_device rd;
    std::default_random_engine randGenerator{rd()};
    // number to direction -> 1=>left, 2=>up, 3=>right, 4=>down
    std::uniform_int_distribution<int> randDistribution(1, 4);

    // Numbers of steps the ant takes in this thread
    unsigned long threadAntSteps = 0;
    // Number of times the problem is run in this thread
    unsigned long threadNumRuns = 0;

    while (threadNumRuns <= numRuns)
    {
        // Starting point of the ant
        int antX = 2, antY = 2;
        // Tracker to check the number of seeds placed in the top row
        int seedsPlaced = 0;

        // Array to denote the seeds at all the cells in the bottom row
        std::array<bool, 5> bottomRowSeeds{ {true, true, true, true, true} };
        // Array to denote the seeds at all the cells in the top row
        std::array<bool, 5> topRowSeeds{ {false, false, false, false, false} };

        // Flag to denote whether the ant is carrying a seed
        bool antHasSeed{ false };

        // Loop to move the ant till all the seeds have been placed
        while (seedsPlaced != 5)
        {
            // Flag to check if the ant has made a valid move
            bool antMoved = false;

            // Loop to try to move the ant in different directions
            while (antMoved == false)
            {
                // Direction the ant attempts to move
                int direction = randDistribution(randGenerator);

                // Check if direction is valid
                if (
                    // if in first column => cannot go left
                    (antX == 0 && direction == 1) ||
                    // if in last column => cannot go  right
                    (antX == 4 && direction == 3) ||
                    // if in the bottom row => cannot go down
                    (antY == 0 && direction == 4) ||
                    // if in the top row => cannot go up
                    (antY == 4 && direction == 2))
                {
                    continue;
                }

                // Valid direction found => move ant
                switch (direction)
                {
                case 1:
                    // direction => left
                    --antX;
                    break;
                case 2:
                    // direction => up
                    ++antY;
                    break;
                case 3:
                    // direction => right
                    ++antX;
                    break;
                case 4:
                    // direction => down
                    --antY;
                    break;
                default:
                    break;
                }
                antMoved = true;
            }
            // The move loop ensures that the ant can only move in valid directions

            // Check if ant in bottom row
            if (antY == 0)
            {
                // Check if:
                // 1. Ant carrying seed?
                // 2. X location has a seed
                if (antHasSeed == false && bottomRowSeeds[antX] == true)
                {
                    // Ant picks up the seed
                    antHasSeed = true;
                    // Removes the seed from the grid
                    bottomRowSeeds[antX] = false;
                }
            }
            // Check if ant in top row
            else if (antY == 4)
            {
                // check if:
                // 1. Ant carring seed?
                // 2. X location does not have seed
                if (antHasSeed == true && topRowSeeds[antX] == false)
                {
                    // Ant keeps the seed in cell
                    antHasSeed = false;
                    // Adds the seed to the grid
                    topRowSeeds[antX] = true;
                    // Increments the top seeds placed tracker
                    ++seedsPlaced;
                }

            }
            // Increment the number of steps the ant has moved for current thread
            ++threadAntSteps;
        }
        // Increment the number of times problem run for this thread
        ++threadNumRuns;
    }
    // Use mutex locks to protect the write to the total stats variables
    mtxStatsWrite.lock();
    // Update the top level total statistics
    totalRuns += threadNumRuns;
    totalSteps += threadAntSteps;
    // Release the lock
    mtxStatsWrite.unlock();
}

int main()
{
    // Number of total runs required
    unsigned long totalRunsRequired = 10000000;

    // Get the number of threads
    unsigned long numThreads(std::thread::hardware_concurrency());

    // adding 1 to the numThreads to account for the main thread
    unsigned long runsPerThread = ceil(totalRunsRequired / numThreads);

    // create vector of threads
    std::vector<std::thread> threadVector;
    threadVector.reserve(numThreads);
;
    // spawn the thread with function
    for (unsigned long i = 0; i < numThreads; ++i)
    {
        threadVector.push_back(std::thread(antTravelGird, runsPerThread));
    }

    // Join the threads to the main thread
    for (unsigned long i = 0; i < numThreads; ++i)
    {
        threadVector[i].join();
    }

    // Open output stream to write data into out file
    std::ofstream ofOutFile;
    ofOutFile.open("ProblemOne.txt", std::ios::trunc);

    if (not ofOutFile.is_open())
    {
        // If unable to open output file => print error
        std::cerr << "Unable to open output file: ProblemOne.txt" << std::endl;
        return 1;
    }

    ofOutFile << "Number of threads created: " << (unsigned long)numThreads << "\n\n";
    ofOutFile << std::fixed << std::setprecision(6) << "Expected number of steps: " << (double)(totalSteps / totalRuns) << "\n\n";
    ofOutFile << "Total number of runs needed for solution convergence: " << (unsigned long)totalRuns;
    ofOutFile.close();

    return 0;
}
