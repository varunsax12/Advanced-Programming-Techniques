
/*
* Implementation file for HelperFunctions.cpp
*/

#include "HelperFunctions.h"

#include <string>
#include <algorithm>
#include <cstring>
#include <iostream>

/*
* Function to check whether the input argument is a number,
* if so, then set the number through reference
* else, return false
*
* @param charsToCheck char array to verify
* @param ulInNumber reference to original variable to set
* Returns: bool if input is a number
*/
bool convertToNumbers(const char* charsToCheck, unsigned short& ulInNumber)
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
            ulInNumber = (unsigned short)std::stoul(strToCheck);

            // Return false if the port number outside range 61000-65535
            if (ulInNumber < 61000 || ulInNumber > 65535)
            {
                return false;
            }

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

/*
* Function to print invalid argument detected message
* 
* @param inArg -> invalid argument to print
*/
void printInvalidArgs(const char* inArg)
{
    std::cout << "Invalid command line argument detected: " << inArg << "\n";
    checkEnterPress();
}

/*
* Function to poll for enter press
*/
void checkEnterPress()
{
    std::cout << "Please check your values and press Enter to end program!\n";
    while (std::cin.get() != '\n')
    {
        std::cout << "Please check your values and press Enter to end program!\n";
    }
}