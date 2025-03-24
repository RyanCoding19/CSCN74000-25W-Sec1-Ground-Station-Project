// GroundTower.cpp

#include "GroundTower.h"
#include "Aircraft.h"

using namespace std;

// Constructor
GroundTower::GroundTower(const string& name, double lat, double lon, double radius)
    : towerName(name), latitude(lat), longitude(lon), operationalRadius(radius), server_fd(INVALID_SOCKET) {}

// Destructor
GroundTower::~GroundTower() {
    if (server_fd != INVALID_SOCKET) {
        closesocket(server_fd);
        WSACleanup();
    }
}


// Register an aircraft (Save the object aircraft into the vector list)
void GroundTower::registerAircraft(const Aircraft& aircraft) {
    aircraftList.push_back(aircraft);
    cout << "Aircraft " << aircraft.aircraftID << " registered at " << towerName << " tower.\n";
}

// Update an aircraft in the vector list of all aircrafts
void GroundTower::updateAircraft(const Aircraft& aircraft) {
    for (auto& existingAircraft : aircraftList) {
        if (existingAircraft.aircraftID == aircraft.aircraftID) {
            existingAircraft.latitude = aircraft.latitude;
            existingAircraft.longitude = aircraft.longitude;
            existingAircraft.altitude = aircraft.altitude;
            existingAircraft.speed = aircraft.speed;
            existingAircraft.fuelLevel = aircraft.fuelLevel;
            cout << "Updated aircraft " << aircraft.aircraftID << "'s information.\n";
            return;
        }
    }
    cout << "Aircraft " << aircraft.aircraftID << " not found!\n";
}

// Display all information regarding the aircraft vector list
void GroundTower::displayAllAircraft() const {
    cout << "Aircraft under " << towerName << " tower:\n";
    for (const auto& aircraft : aircraftList) {
        cout << " - Aircraft ID: " << aircraft.aircraftID
            << " | Location: (" << aircraft.latitude << ", " << aircraft.longitude << ")"
            << " | Altitude: " << aircraft.altitude << " meters"
            << " | Speed: " << aircraft.speed << " km/h"
            << " | Fuel Level: " << aircraft.fuelLevel << "%\n";
    }
}


// Listen for new connections to the socket
void GroundTower::listenForAircraft() {
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080); // Port Number

    // Bind the socket
    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        cerr << "Bind failed: " << WSAGetLastError() << endl;
        return;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        cerr << "Listen failed: " << WSAGetLastError() << endl;
        return;
    }

    cout << "Tower " << towerName << " is now listening for aircraft...\n";

    SOCKET client_socket;
    sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);

    // Continuously accept new aircraft connections
    while (true) {
        client_socket = accept(server_fd, (sockaddr*)&client_addr, &addrlen);
        if (client_socket == INVALID_SOCKET) {
            cerr << "Accept failed: " << WSAGetLastError() << endl;
            return;
        }

        // Handle communication with the connected aircraft
        thread communicationThread(&GroundTower::handleAircraftCommunication, this, client_socket);
        communicationThread.detach();  // Detach to handle multiple aircraft concurrently
    }
}


// Handle the data transmissions
void GroundTower::handleAircraftCommunication(SOCKET client_socket) {
    char buffer[1024] = { 0 };

    // Receive the message from the aircraft
    int bytesReceived = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';  // Null-terminate the received data
        cout << "Received from aircraft: " << buffer << endl;

        // Parse the message
        string msg(buffer);
        size_t pos = 0;
        string token;
        string aircraftID;
        double lat, lon, alt, spd, fuel;

        // Format: "ID,latitude,longitude,altitude,speed,fuel"
        if ((pos = msg.find(',')) != string::npos) {
            aircraftID = msg.substr(0, pos);
            msg.erase(0, pos + 1);
        }
        if ((pos = msg.find(',')) != string::npos) {
            lat = stod(msg.substr(0, pos));
            msg.erase(0, pos + 1);
        }
        if ((pos = msg.find(',')) != string::npos) {
            lon = stod(msg.substr(0, pos));
            msg.erase(0, pos + 1);
        }
        if ((pos = msg.find(',')) != string::npos) {
            alt = stod(msg.substr(0, pos));
            msg.erase(0, pos + 1);
        }
        if ((pos = msg.find(',')) != string::npos) {
            spd = stod(msg.substr(0, pos));
            msg.erase(0, pos + 1);
        }
        fuel = stod(msg);

        // Check if the aircraft is already registered
        Aircraft newAircraft(aircraftID, lat, lon, alt, spd, fuel);
        bool aircraftFound = false;
        for (const auto& aircraft : aircraftList) {
            if (aircraft.aircraftID == aircraftID) {
                aircraftFound = true;
                break;
            }
        }

        // Register or update aircraft
        if (!aircraftFound) {
            registerAircraft(newAircraft);  // Register new aircraft
        }
        else {
            updateAircraft(newAircraft);    // Update existing aircraft
        }
    }

    // Send a response to the aircraft
    const char* response = "Message received by Ground Tower!";
    send(client_socket, response, strlen(response), 0);

    closesocket(client_socket);
}


// Start the Ground Tower Server
void GroundTower::startListening() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cerr << "WSAStartup failed: " << iResult << endl;
        return;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        cerr << "Socket failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return;
    }

    // Start listening for aircraft on a separate thread
    thread listenerThread(&GroundTower::listenForAircraft, this);
    listenerThread.detach();  // Detach to keep listening for new aircraft
}