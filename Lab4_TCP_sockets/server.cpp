/*
Description:
    Solution file for Lab 4.
    Server side program.
*/


#include <iostream>
#include <cstdlib>
#include <list>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>

// SFML includes
#include <SFML/Network.hpp>

// Custom header files
#include "HelperFunctions.h"

std::mutex mtxIsEnterPressed;
bool isEnterPressed = false;

/*
* Function to write the message into server.log
* 
* @param message -> message to write into the log
*/
void writeToServerLog(std::string& message)
{
    std::ofstream serverLog;
    serverLog.open("server.log", std::ios::app);
    serverLog << message << std::endl;
    serverLog.close();
}

/*
* Function to check for enter press and update the thread variable so that
* server thread can read it
*/
void checkEnterPressThread()
{
    while (std::cin.get() != '\n')
    {
        continue;
    }

    // Update the enter pressed flag so that the server can now stop listening for ports
    mtxIsEnterPressed.lock();
    isEnterPressed = true;
    mtxIsEnterPressed.unlock();
}

/*
* Function to start the tcp server and listen for client connections
* and detect messages sent by the client
* 
* @param port -> Port number to listen to
* 
* Used the below reference to use the sfml selector to switch between
* the listening port and the clients
* Reference: https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1SocketSelector.php
*/
void runTcpServer(unsigned short inPortNumber)
{
    // Selector to switch between the listener and the clients
    sf::SocketSelector socketSelector;

    // List to store all the clients connecting the server
    std::list<sf::TcpSocket*> listClients;

    // Start the listener on the server port
    sf::TcpListener listener;
    // Start listening to the server port
    sf::Socket::Status status = listener.listen(inPortNumber);
    if (status != sf::Socket::Done)
    {
        std::cout << "Failed to start server on port number: " << inPortNumber << "\n";
        std::cout << "Please check your values and press Enter to end program!\n";
        return;
    }
    // Add the listener to the socket selector to context switch
    socketSelector.add(listener);

    // Variables to pass the message
    std::string message = "";

    // Set a refresh time to poll for enter press
    sf::Time timeout = sf::microseconds(1);

    // Loop to continuously check the listener and clients
    while (true)
    {
        // Check if enter was pressed
        if (isEnterPressed)
        {
            // enter pressed => server must exit
            // delete all clients
            for (auto client : listClients)
            {
                delete client;
            }

            // Print exit message
            std::cout << "Enter pressed. Exiting server.\n";
            return;
        }

        // Wait for any change in sockets added to the selector
        if (socketSelector.wait(timeout))
        {
            // Check for changes in the listener
            if (socketSelector.isReady(listener))
            {
                // Change in listener => could be a new client attempting to connect
                // Using pointers as SFML does not support copying clients
                sf::TcpSocket* client = new sf::TcpSocket;
                if (listener.accept(*client) == sf::Socket::Done)
                {
                    // Add the new client to the client list to track when checking for responses in clients
                    listClients.push_back(client);
                    // Add the client to the selector so that the selector can track for changes in this client
                    // as well
                    socketSelector.add(*client);
                    
                    // Pass the message to be printed into server log
                    message = "Client connected";
                    writeToServerLog(message);
                }
                else
                {
                    // If the listener update was not a client connecting => delete the pointer
                    delete client;
                }
            }
            // Check for changes in the clients
            else
            {
                // Track clients to remove from client list
                std::list<sf::TcpSocket*> listClientsToRemove;

                // Check to find which client triggered the response
                for (std::list<sf::TcpSocket*>::iterator itr = listClients.begin(); itr != listClients.end(); ++itr)
                {
                    sf::TcpSocket& currentClient = **itr;
                    if (socketSelector.isReady(currentClient))
                    {
                        // Current client is sending some data => read message (max size 1000 as per Piazza post)
                        const int inMessageSize = 1000;
                        char inMessage[inMessageSize];
                        std::size_t received;

                        // Check client status
                        sf::Socket::Status status = currentClient.receive(inMessage, inMessageSize, received);

                        // Check if client disconnected
                        if (status == sf::Socket::Disconnected)
                        {
                            // Pass message to be printed into server log
                            message = "Client disconnected";
                            writeToServerLog(message);
                            // Client has disconnected => remove from client list
                            // Add to new list to remove eventually
                            listClientsToRemove.push_back(*itr);
                        }
                        else if (status == sf::Socket::Done)
                        {
                            // Convert the message into string type
                            message = inMessage;
                            // Pass message to be printed into server log
                            writeToServerLog(message);
                        }
                    }
                }

                // Check for and remove the clients from the client list
                for (std::list<sf::TcpSocket*>::iterator itr = listClientsToRemove.begin(); itr != listClientsToRemove.end(); ++itr)
                {
                    // Remove from the client list
                    listClients.remove(*itr);
                    // Disconnect socket from selector
                    socketSelector.remove(**itr);
                    // Deallocate the pointer space
                    delete* itr;
                }
            }
        }
    }
}

int main(int argc, char* argv[])
{
    // Check for command line port number
    if (argc == 1)
    {
        std::cout << "Invalid command line argument detected: missing portnumber!\n";
        checkEnterPress();
        return 0;
    }
    else if (argc > 2)
    {
        // args greater than 2
        std::cout << "Invalid command line argument detected: ";
        for (int i = 2; i < argc; ++i)
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
    if (not convertToNumbers(argv[1], portNumber))
    {
        // Check 2: Print error if the input is not a number
        printInvalidArgs(argv[1]);
        return 1;
    }

    // Run the server and cin check on separate threads
    std::thread cinThread = std::thread(checkEnterPressThread);
    std::thread serverThread = std::thread(runTcpServer, portNumber);

    serverThread.join();
    cinThread.join();

    return 1;
}
