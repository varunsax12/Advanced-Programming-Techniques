/*
* Header file for helper functions
*/

#ifndef __HELPERFUNCTIONS__HEADER__
#define __HELPERFUNCTIONS__HEADER__

/*
* Function to check whether the input argument is a number,
* if so, then set the number through reference
* else, return false
*
* @param charsToCheck char array to verify
* @param ulInNumber reference to original variable to set
* Returns: bool if input is a number
*/
bool convertToNumbers(const char* charsToCheck, unsigned short& ulInNumber);

/*
* Function to print invalid argument detected message
* 
* @param inArg -> invalid argument to print
*/
void printInvalidArgs(const char* inArg);

/*
* Function to poll for enter press
*/
void checkEnterPress();

#endif // !__HELPERFUNCTIONS__HEADER__

