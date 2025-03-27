#include "Aircraft.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define SERVER_IP "127.0.0.1"

// Function to send aircraft data
void sendAircraftData(SOCKET sock, Aircraft& aircraft) {
    while (true) {
        aircraft.fuelLevel -= 0.5;
        if (aircraft.fuelLevel < 0) aircraft.fuelLevel = 0;

        // Construct message as comma-separated values
        std::string message = aircraft.aircraftID + "," +
            std::to_string(aircraft.latitude) + "," +
            std::to_string(aircraft.longitude) + "," +
            std::to_string(aircraft.altitude) + "," +
            std::to_string(aircraft.speed) + "," +
            std::to_string(aircraft.fuelLevel);

        // Send data
        if (send(sock, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
            std::cerr << "Error sending data: " << WSAGetLastError() << std::endl;
            break;
        }
        std::cout << "Sent: " << message << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

// Function to receive messages from the server
void receiveServerMessages(SOCKET sock) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "Server: " << buffer << std::endl;
        }
        else {
            std::cerr << "Connection lost or error receiving data" << std::endl;
            break;
        }
    }
}

int main() {
    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in serv_addr{};

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return 1;
    }

    while (true) {
        // Create socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            std::cerr << "Socket creation failed!" << std::endl;
            WSACleanup();
            return 1;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);
        inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

        // Attempt connection
        if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
            std::cerr << "Connection failed! Retrying..." << std::endl;
            closesocket(sock);
            std::this_thread::sleep_for(std::chrono::seconds(3));
            continue;  // Retry connection
        }

        std::cout << "Connected to ground control!" << std::endl;

        // Initialize aircraft data
        Aircraft aircraft("Aircraft1", 40.7128, -74.0060, 10000, 800, 100);

        // Start sender and receiver threads
        std::thread sender(sendAircraftData, sock, std::ref(aircraft));
        std::thread receiver(receiveServerMessages, sock);

        sender.join();
        receiver.join();

        closesocket(sock);
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    WSACleanup();
    return 0;
}