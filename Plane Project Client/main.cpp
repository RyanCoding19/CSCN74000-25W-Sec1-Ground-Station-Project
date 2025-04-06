#include "Aircraft.h"
#include "ClientHeader.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <random>
#include <mutex>
#include <atomic>
#include <cstdlib>
#include <future>

namespace MainClient {

    constexpr uint16_t PORT = 8080;
    constexpr const char* SERVER_IP = "127.0.0.1";
    constexpr size_t BUFFER_SIZE = 1024;
    constexpr double FUEL_CONSUMPTION_RATE = 0.5;
    constexpr std::chrono::seconds DATA_SEND_INTERVAL(5);

    std::atomic<bool> g_keepRunning{ true };
    std::mutex g_aircraftMutex;

    enum class ErrorCode {
        SUCCESS = 0,
        WINSOCK_INIT_FAILED,
        SOCKET_CREATION_FAILED,
        CONNECTION_FAILED,
        SEND_FAILED,
        RECEIVE_FAILED
    };

    // Function to send aircraft data
    void sendAircraftData(SOCKET socket, PlaneSystem::Aircraft& aircraft) {
        while (g_keepRunning) {
            // Update of aircraft state
            {
                std::lock_guard<std::mutex> lock(g_aircraftMutex);

                double currentFuel = aircraft.GetFuelLevel();
                currentFuel -= FUEL_CONSUMPTION_RATE;
                aircraft.SetFuelLevel(currentFuel < 0.0 ? 0.0 : currentFuel);
            }

            // Construct message as comma-separated values
            std::string message;
            {
                std::lock_guard<std::mutex> lock(g_aircraftMutex);

                message = aircraft.GetAircraftID() + "," +
                    std::to_string(aircraft.GetLatitude()) + "," +
                    std::to_string(aircraft.GetLongitude()) + "," +
                    std::to_string(aircraft.GetAltitude()) + "," +
                    std::to_string(aircraft.GetSpeed()) + "," +
                    std::to_string(aircraft.GetFuelLevel());
            }

            // Send data
            if (send(socket, message.c_str(), static_cast<int>(message.length()), 0) == SOCKET_ERROR) {
                std::cerr << "Error sending data: " << WSAGetLastError() << std::endl;
                g_keepRunning = false;
                break;
            }
            std::cout << "Sent: " << message << std::endl;

            // Sleep between updates
            std::this_thread::sleep_for(DATA_SEND_INTERVAL);
        }
    }

    // Function to receive messages from the server
    void receiveServerMessages(SOCKET socket) {
        char buffer[BUFFER_SIZE];
        while (g_keepRunning)
        {
            // Clear buffer before receiving new data
            (void)std::memset(buffer, 0, BUFFER_SIZE);

            int bytesReceived = recv(socket, buffer, BUFFER_SIZE - 1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';   // Null-terminate the received data

                std::string message(buffer);
                // Check if this is a command message (starts with CMD:)
                if (message.substr(0, 4) == "CMD:") {
                    std::cout << "\n=== GROUND CONTROL COMMAND ===\n";
                    std::cout << message.substr(4) << std::endl;
                    std::cout << "==============================\n";
                }
                else {
                    std::cout << "Server: " << buffer << std::endl;
                }
            }
            else if (bytesReceived == 0) {
                std::cout << "Connection closed by server" << std::endl;
                g_keepRunning = false;
                
            }
            else {
                std::cerr << "Error receiving data: " << WSAGetLastError() << std::endl;
                g_keepRunning = false;
                break;
            }
        }
    }

    // Function to generate a random aircraft ID
    std::string generateAircraftID() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000, 9999);

        return "Aircraft-" + std::to_string(dis(gen));
    }

    // Clean up socket resources
    void cleanupSocket(SOCKET& socket) {
        if (socket != INVALID_SOCKET) {
            (void)closesocket(socket);
            socket = INVALID_SOCKET;
        }
    }

}

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    SOCKET clientSocket = INVALID_SOCKET;
    sockaddr_in serverAddr{};

    int returnresult = 0;

    // Parse command arguments for aircraft ID
    std::string aircraftID;
    if (argc > 1) {
        aircraftID = argv[1];
    }
    else {
        aircraftID = MainClient::generateAircraftID();
    }

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        returnresult = static_cast<int>(MainClient::ErrorCode::WINSOCK_INIT_FAILED);
    }

    // Generate randomized starting position
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<> latDist(40.7000, 40.7300);
    std::uniform_real_distribution<> lonDist(-74.0200, -73.9800);
    std::uniform_real_distribution<> altDist(9000.0, 11000.0);
    std::uniform_real_distribution<> spdDist(750.0, 850.0);

    // Create aircraft object with safe initialization
    PlaneSystem::Aircraft aircraft(
        aircraftID,
        latDist(gen),
        lonDist(gen),
        altDist(gen),
        spdDist(gen),
        100.0  // Full fuel
    );

    std::cout << "Aircraft " << aircraft.GetAircraftID() << " initialized." << std::endl;
    std::cout << "Position: (" << aircraft.GetLatitude() << ", " << aircraft.GetLongitude() << ")" << std::endl;
    std::cout << "Altitude: " << aircraft.GetAltitude() << " meters" << std::endl;
    std::cout << "Speed: " << aircraft.GetSpeed() << " km/h" << std::endl;

    bool mainloop = true;
    while (mainloop) {
        MainClient::g_keepRunning = true;

        // Create socket 
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
            (void)WSACleanup();
            if (returnresult == 0) {
                returnresult = static_cast<int>(MainClient::ErrorCode::SOCKET_CREATION_FAILED);
            }
            break;
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(MainClient::PORT);
        (void)inet_pton(AF_INET, MainClient::SERVER_IP, &serverAddr.sin_addr);

        // Attempt connection 
        std::cout << "Attempting to connect to ground control tower..." << std::endl;

        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Connection failed with error: " << WSAGetLastError() << std::endl;
            MainClient::cleanupSocket(clientSocket);
            std::cout << "Retrying in 3 seconds..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(3));
            continue;
        }

        std::cout << "Connected to ground control!" << std::endl;
        std::cout << "Press Enter to disconnect, or wait for updates..." << std::endl;

        // Start sender and receiver threads
        std::thread sender(MainClient::sendAircraftData, clientSocket, std::ref(aircraft));
        std::thread receiver(MainClient::receiveServerMessages, clientSocket);

        // Use a future to wait for user input 
        std::future<void> userInput = std::async(std::launch::async, [&]() {
            std::string input;
            (void)std::getline(std::cin, input);

            MainClient::g_keepRunning = false;

            // Shutdown the socket to signal disconnection to the server
            if (clientSocket != INVALID_SOCKET) {
                (void)shutdown(clientSocket, SD_SEND); // Signal we're done sending
                std::cout << "Connection closed by client (Aircraft " << aircraft.GetAircraftID() << ")" << std::endl;
            }
            });

        // Wait for either threads to finish or user to press Enter
        if (sender.joinable()) {
            sender.join();
        }

        if (receiver.joinable()) {
            receiver.join();
        }

        std::cout << "Disconnected from ground control tower." << std::endl;

        MainClient::cleanupSocket(clientSocket);

        std::cout << "Reconnect? (y/n): ";
        std::string reconnect;
        (void)std::getline(std::cin, reconnect);

        if (reconnect != "y" && reconnect != "Y") {
            std::cout << "Exiting program." << std::endl;
            mainloop = false;
           
        }

        if (reconnect == "y" && reconnect == "Y") {
            std::cout << "Reconnecting in 3 seconds..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }

    (void)WSACleanup();
    if (returnresult == 0) {
        returnresult = static_cast<int>(MainClient::ErrorCode::SUCCESS);
    }

    return returnresult;
}
