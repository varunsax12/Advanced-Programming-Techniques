/*
Description:
    Solution file for Problem 2 in Lab 1
    Ant is moving around in the infinite grid starting from 0,0
    With each step, it takes a turn and moves around, flipping the color
    of the cells it was on.
*/

#include <algorithm>
#include <cstring>
#include <iostream>
#include <fstream>
#include <set>
#include <string>

/* 
* Global set of pairs to track the cells which are black
* Pair
* first => x coordinate
* second => y coordinate
*/
std::set<std::pair<unsigned long, unsigned long>> glBlackCells;

/*
* Global enum of directions the ant can go in
*/
enum direction {left, right, up, down};

/*
* Function to check whether the input argument is a number,
* if so, then set the number through reference
* else, return false
*
* @param strToCheck char array to verify
* @param inNumber reference to original variable to set
* Returns: bool if input is a number
*/
bool convertToNumber(const char* charsToCheck, unsigned long& inNumber)
{
    try
    {
        // check to see if the char array is a number
        // Using lambda function to check if each char is a number
        bool check = std::all_of(charsToCheck, charsToCheck + strlen(charsToCheck),
            [](unsigned char c) { return ::isdigit(c); });
        if (check)
        {
            std::string strToCheck(charsToCheck);
            inNumber = std::stoul(strToCheck);
            if (inNumber >= 0)
            {
                return true;
            }
        }
        return false;
    }
    catch (const std::exception&)
    {
        // return invalid if exception occured
        return false;
    }
}

/*
* Function to check whether cell is black
*
* @param ulX x coordinate
* @param ulY y coordinate
* Return bool: whether the x,y inputs is a black cell
*/
bool isBlack(const unsigned long ulX, const unsigned long ulY)
{
    std::pair<unsigned long, unsigned long> pCurrentLoc = std::make_pair(ulX, ulY);
    auto itr = glBlackCells.find(pCurrentLoc);
    return itr != glBlackCells.end();
}

/*
* Function to rotate the ant
* 
* @param ulX x coordinate of ant
* @param ulY y coordinate of ant
* @param currDirection current direction of the ant
* 
* Reference updates:
* ulX, ulY to the new x and y coorindates
* currDirection to the new direction
*/
void rotateAnt(unsigned long &ulX, unsigned long &ulY, direction &currDirection)
{
    // check if current cell is black
    if (isBlack(ulX, ulY))
    {
        // current cell => black

        // rotate counter clock wise
        if (currDirection == left)
        {
            currDirection = down;
            ulY--;
        }
        else if (currDirection == down)
        {
            currDirection = right;
            ulX++;
        }
        else if (currDirection == right)
        {
            currDirection = up;
            ulY++;
        }
        else // up
        {
            currDirection = left;
            ulX--;
        }
    }
    else
    {
        // current cell => white

        // rotate clock wise
        if (currDirection == left)
        {
            currDirection = up;
            ulY++;
        }
        else if (currDirection == up)
        {
            currDirection = right;
            ulX++;
        }
        else if (currDirection == right)
        {
            currDirection = down;
            ulY--;
        }
        else // down
        {
            currDirection = left;
            ulX--;
        }
    }
}

/*
* Function to flip the current coordinate
* 
* @param x x coordinate of ant
* @param y y coordinate of ant
* 
* Function updates: Adds or removes teh cell from global black cells set
*/
void flipCell(const unsigned long ulX, const unsigned long ulY)
{
    std::pair<unsigned long, unsigned long> pCurrentLoc = std::make_pair(ulX, ulY);

    // look for x,y in the global blacks cells set
    auto itr = glBlackCells.find(pCurrentLoc);

    if (itr != glBlackCells.end())
    {
        // current location was previously black
        // => Remove from the black cells set as now it will become white
        glBlackCells.erase(itr);
    }
    else
    {
        // current location was previously white
        // Add to black cells set as now it will become black
        glBlackCells.insert(pCurrentLoc);
    }
}

/*
* Function to move ants for the required number of steps
* 
* @param ulNumSteps steps to be taken by the ant
*/
void moveAnt(unsigned long ulNumSteps)
{
    // starting point
    unsigned long ulX{ 0 }, ulY{ 0 };
    // starting direction
    direction currDirection{ left };
    while (ulNumSteps > 0)
    {
        // iterate through each step

        // rotate the ant
        rotateAnt(ulX, ulY, currDirection);
        // flip the cell color
        flipCell(ulX, ulY);
        // Decrememnt the number of steps left to take
        ulNumSteps--;
    }
}

/*
* Main function of the program
* @param argc Number of input arguments
* @param argv Char array of the input arguments
*/
int main(int argc, char* argv[])
{
    // Write data into file
    std::ofstream outfile;
    outfile.open("output2.txt", std::ios::trunc);

    if (not outfile.is_open())
    {
        // Print error if unable to open the file
        std::cerr << "Unable to open output file: output2.txt" << std::endl;
        return 1;
    }

    // Check 1: Expected input args = 1
    // If args = 2
    if (argc != 2)
    {
        // Check 1: Expected input args = 1 (+ executable)
        // Print error if does not match check
        outfile << "Invalid inputs";
        outfile.close();
        return 1;
    }

    // Variable to hold the input number
    unsigned long inNumber{ 0 };

    if (not convertToNumber(argv[1], inNumber))
    {
        // Check 2: Input number should be a number
        // Print erorr is not a number
        outfile << "Invalid inputs";
        outfile.close();
        return 1;
    }

    // Move the ant around
    moveAnt(inNumber);

    // Check if the file is open
    outfile << glBlackCells.size();
    outfile.close();

    return 0;
}
