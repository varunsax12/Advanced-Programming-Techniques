/*
Description:
    Solution file for Lab 4.
    Client side program.
*/

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>

// SFML includes
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>

// Custom header file
#include "HelperFunctions.h"

/*
* Function to print the failed to connect message
* @param inIpAddress -> input ip address
* @param portNumer -> input port number
*/
void printFailedToConnect(std::string inIpAddress, unsigned short portNumber)
{
    std::cout << "Failed to connect to the server at " << inIpAddress << " on " << portNumber << "\n";
    checkEnterPress();
}

/*
* Function to start a tcp client
* @param inIpAddress -> input ip address
* @param portNumber -> port number to connect to
*/
void runTcpClient(std::string inIpAddress, unsigned short portNumber)
{
    // Check for validity of the ip address
    if (inIpAddress == sf::IpAddress::None)
    {
        // invalid IP address => print the error message and return
        printFailedToConnect(inIpAddress, portNumber);
        return;
    }

    // Create a socket for communicating with the server
    sf::TcpSocket socket;

    // Connect to the server
    if (socket.connect(inIpAddress, portNumber) != sf::Socket::Done)
    {
        // Unable to connect to server => print error message and return
        printFailedToConnect(inIpAddress, portNumber);
        return;
    }

    // Run the while loop and keep asking for new messages to send
    while (true)
    {
        std::cout << "Please enter a message:";
        char message[1000];
        std::cin.getline(message, 1000);

        if (strlen(message) == 0)
        {
            std::cout << "No data input and enter pressed. Exiting program\n";
            socket.disconnect();
            return;
        }

        // Send an answer to the server
        if (socket.send(message, 1000) != sf::Socket::Done)
        {
            // Unable to send message => print error
            std::cout << "Unable to send message to server at " << inIpAddress << " port " << portNumber << "\n";
        }
    }
}

int main(int argc, char* argv[])
{
    // Check for command line port number and ip address
    if (argc == 1)
    {
        std::cout << "Invalid command line argument detected: missing ipaddress and portnumber!\n";
        checkEnterPress();
        return 0;
    }
    else if (argc == 2)
    {
        std::cout << "Invalid command line argument detected: missing portnumber!\n";
        checkEnterPress();
        return 0;
    }
    else if (argc > 3)
    {
        std::cout << "Invalid command line argument detected: ";
        for (int i = 3; i < argc; ++i)
        {
            std::cout << argv[i];
            if (i != argc - 1)
            {
                std::cout << ", ";
            }
        }
        std::cout << "\n";
        checkEnterPress();
        return 0;
    }

    // Variable to hold the port number to listen to
    unsigned short portNumber{ 0 };

    // Convert the char array into the required number
    if (not convertToNumbers(argv[2], portNumber))
    {
        // Check 2: Print error if the input is not a number
        printInvalidArgs(argv[2]);
        return 1;
    }

    std::string inIpAddress = argv[1];

    // Run the tcp client
    runTcpClient(inIpAddress, portNumber);

    return 1;
}
