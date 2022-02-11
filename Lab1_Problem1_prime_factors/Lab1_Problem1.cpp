/*
Description:
    Solution file for Problem 1 in Lab 1.
    Uses function as described in the lab PDF to check for prime factors.
    Output: Printed in output1.txt
*/

#include <algorithm>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>

/*
* Function to check whether the input argument is a number, 
* if so, then set the number through reference
* else, return false
* 
* @param strToCheck char array to verify
* @param inNumber reference to original variable to set
* Returns: bool if input is a number
*/
bool convertToNumbers(const char* charsToCheck, unsigned long& ulInNumber)
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
            ulInNumber = std::stoul(strToCheck);
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
* Function to check for prime factors
* 
* @param ulInputNumber input number
* @param strOutput reference to output string to update
* Returns: bool if prime factors exists
*/
bool GetPrimeFactors(const unsigned long ulInputNumber, std::string& strOutput)
{
    bool bPrimeFactorExists = false;
    unsigned long ulInputNumberCopy = ulInputNumber;

    while (ulInputNumberCopy % 2 == 0)
    {
        // Check whether the number has 2 as a prime factor
        // and keep checking till 2 is no longer a prime factor
        strOutput += "2,";
        ulInputNumberCopy /= 2;
        bPrimeFactorExists = true;
    }

    // After 2 is checked, all other factors should be odd

    for (unsigned long ll = 3; ulInputNumberCopy > 1; ll++)
    {
        // Check whether number has ll as the prime factor
        // and keep checking till ll is no longer a prime factor
        while (ulInputNumberCopy % ll == 0)
        {
            strOutput += std::to_string(ll) + ",";
            ulInputNumberCopy /= ll;
            bPrimeFactorExists = true;
        }
    }

    // Remove the last commma
    strOutput = strOutput.substr(0, strOutput.length() - 1);
    return bPrimeFactorExists;
}

/*
* Main function of the program
* 
* Checks for the validity of the command line argument
* Calls function GetPrimeFactors to get the prime factor string
* Prints the output in output1.txt
* 
* @param argc Number of arguments provided in the command line
* @param argv Command line arguments char array
*/
int main(int argc, char* argv[])
{
    // Open output stream to write data into out file
    std::ofstream ofOutFile;
    ofOutFile.open("output1.txt", std::ios::trunc);

    if (not ofOutFile.is_open())
    {
        // If unable to open output file => print error
        std::cerr << "Unable to open output file: output1.txt" << std::endl;
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

    // Variable to hold the input number
    unsigned long ulInNumber{ 0 };

    if (not convertToNumbers(argv[1], ulInNumber))
    {
        // Check 2: Print error if the input is not a number
        ofOutFile << "Invalid inputs";
        ofOutFile.close();
        return 1;
    }

    // Variable to hold the expected output
    std::string strOutput("");

    if (ulInNumber == 0 || ulInNumber == 1 || not GetPrimeFactors(ulInNumber, strOutput))
    {
        // Check 3:
        //  a. number == 0 => no prime factors
        //  b. number == 1 => no prime factors
        //  c. If number does not have any prime factors
        // if no prime factor, then enter msgs
        strOutput = "No prime factors";
    }

    // Print output to the file and close
    ofOutFile << strOutput;
    ofOutFile.close();

    return 0;
}
