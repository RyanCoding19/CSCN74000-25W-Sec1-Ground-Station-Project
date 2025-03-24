#include "ClientHeader.h"
#include <iostream>
#include <cstring>  /
#include <string>   

#define PORT 8080

int main() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Example: Aircraft data (no class, just raw data)
    std::string aircraftID = "Aircraft1";
    double latitude = 40.7128;
    double longitude = -74.0060;
    double altitude = 10000;
    double speed = 800;
    double fuelLevel = 50;

    // Construct message as a comma-separated string
    std::string message = aircraftID + "," +
        std::to_string(latitude) + "," +
        std::to_string(longitude) + "," +
        std::to_string(altitude) + "," +
        std::to_string(speed) + "," +
        std::to_string(fuelLevel);

    // Send aircraft data to the server
    send(sock, message.c_str(), message.length(), 0);
    std::cout << "Aircraft data sent: " << message << std::endl;

    char buffer[1024] = { 0 };
    int bytesReceived = recv(sock, buffer, 1024, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';  // Null-terminate the received data
        std::cout << "Server response: " << buffer << std::endl;
    }
    else {
        std::cerr << "Failed to receive response from server" << std::endl;
    }

    closesocket(sock);
    WSACleanup();

    return 0;
}
