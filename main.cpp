#include <iostream>
#include <thread>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
//#include "handleMsg.h"

const int PORT = 12345;
const int BUFFER_SIZE = 1024;
const int MAX_NUM_SOCKET = 4;

void receiveMessages(int socket, int* clientsSocket) {
    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(socket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            std::cerr << "Connection closed by the other user." << std::endl;
            break;
        }

        std::cout << "Received: " << buffer << std::endl;
        for(int i = 0; i < MAX_NUM_SOCKET; i++){
            if(clientsSocket[i] != socket && clientsSocket[i] != -1 && clientsSocket[i] != 0)
                send(clientsSocket[i], buffer, strlen(buffer), 0);
        }
    }
}

void handleClient(int socket, int* clientsSocket, int* clientNum){
    std::cout << "Connection established. You can start typing messages." << std::endl;

    std::thread receiveThread(receiveMessages, socket, clientsSocket);

    receiveThread.join();

    close(socket);
    *clientNum -= 1;
    std::cout << *clientNum << std::endl;
}

int main(int argc, char*argv[]) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    int clientNum = 0;
    std::thread* clients = new std::thread[4];
    int* clientsSocket = new int[4];

    if (serverSocket == -1) {
        std::cerr << "Error creating socket." << std::endl;
        return -1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    if(argc < 2)
      serverAddress.sin_port = htons(PORT);
    else
      serverAddress.sin_port = htons(atoi(argv[1]));
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding socket." << std::endl;
        close(serverSocket);
        return -1;
    }

    if (listen(serverSocket, 1) == -1) {
        std::cerr << "Error listening on socket." << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "Waiting for the other user to connect..." << std::endl;

    // make this able to handle two connection
    sockaddr_in clientAddress{};
    socklen_t clientAddressSize = sizeof(clientAddress);
    int clientSocket = 0;
    do{
								clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddressSize);
								if (clientSocket == -1 && clientNum == MAX_NUM_SOCKET) {
												std::cerr << "Error accepting connection." << std::endl;
												//close(serverSocket);
												//return -1;
            continue;
								}
        
        std::cout << clientNum << std::endl; 
        clientsSocket[clientNum] = clientSocket;

        clients[clientNum] = std::thread(handleClient, clientSocket, clientsSocket, &clientNum);
        clientNum++;
    }while(clientNum > 0);
    // change this to a thread
    /*std::cout << "Connection established. You can start typing messages." << std::endl;

    std::thread receiveThread(receiveMessages, clientSocket);
    std::thread sendThread(sendMessages, clientSocket);

    receiveThread.join();
    sendThread.join();
    */
    close(serverSocket);
    //close(clientSocket);
    clients[0].join();
    clients[1].join();
    delete[] clientsSocket;
    delete[] clients;
    return 0;
}
