// GroundTower.h
#ifndef GROUNDTOWER_H
#define GROUNDTOWER_H

#include <string>
#include <vector>
#include <thread>
#include <winsock2.h> 
#include "Aircraft.h" // Include the Aircraft class
#include <iostream>
#include <cstring> 

using namespace std;

class GroundTower {
private:
    string towerName; // Tower's name
    double latitude; // Tower's latitude
    double longitude; // Tower's longitude
    vector<Aircraft> aircraftList; // List of aircrafts connected to the tower
    double operationalRadius; // in kilometers
    SOCKET server_fd; // Server socket

    // Method to start listening for aircraft
    void listenForAircraft();

    // Helper method to handle aircraft communication
    void handleAircraftCommunication(SOCKET client_socket);

public:
    // Constructor
    GroundTower(const string& name, double lat, double lon, double radius);

    // Destructor to clean up resources
    ~GroundTower();

    // Register an aircraft
    void registerAircraft(const Aircraft& aircraft);

    // Update the aircraft location and metrics
    void updateAircraft(const Aircraft& aircraft);

    // Display all registered aircraft
    void displayAllAircraft() const;

    // Start listening for aircraft
    void startListening();
};

#endif // GROUNDTOWER_H
