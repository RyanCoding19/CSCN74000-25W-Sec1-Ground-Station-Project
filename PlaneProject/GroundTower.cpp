// GroundTower.cpp

#include "GroundTower.h"
#include "Aircraft.h"
#include "ServerHeader.h"

namespace PlaneSystem {
    // Constructor
    GroundTower::GroundTower(const std::string& name, double lat, double lon, double radius)
        : m_towerName(name),
        m_latitude(lat),
        m_longitude(lon),
        m_operationalRadius(radius),
        m_server_fd(INVALID_SOCKET),
        m_isListening(false),
        m_listenerThread(nullptr)
    {
    }

    // Destructor
    GroundTower::~GroundTower() {
        StopListening();
    }

    // Register an aircraft
    void GroundTower::RegisterAircraft(const Aircraft& aircraft) {
        std::lock_guard<std::mutex> lock(m_aircraftMutex);

        //Save the object aircraft into the vector list
        m_aircraftList.push_back(aircraft);
        std::cout << "Aircraft " << aircraft.GetAircraftID() << " registered at " << m_towerName << " tower.\n";
    }

    // Update an aircraft in the vector list of all aircrafts
    void GroundTower::UpdateAircraft(const Aircraft& aircraft) {
        std::lock_guard<std::mutex> lock(m_aircraftMutex);

        auto it = std::find_if(m_aircraftList.begin(), m_aircraftList.end(), [&](const Aircraft& existingAircraft) {
                return existingAircraft.GetAircraftID() == aircraft.GetAircraftID();
            });

        if (it != m_aircraftList.end()) {
            *it = aircraft;
            std::cout << "Updated aircraft " << aircraft.GetAircraftID() << "'s information.\n";
        }
        else {
            std::cout << "Aircraft " << aircraft.GetAircraftID() << " not found!\n";
        }
    }

    // Display all information regarding the aircraft vector list
    void GroundTower::DisplayAllAircraft() const {
        std::lock_guard<std::mutex> lock(m_aircraftMutex);

        std::cout << "Aircraft under " << m_towerName << " tower:\n";
        for (const auto& aircraft : m_aircraftList) {
            std::cout << " - Aircraft ID: " << aircraft.GetAircraftID()
                << " | Location: (" << std::fixed << std::setprecision(6) << aircraft.GetLatitude() << ", " << aircraft.GetLongitude() << ")"
                << " | Altitude: " << aircraft.GetAltitude() << " meters"
                << " | Speed: " << aircraft.GetSpeed() << " km/h"
                << " | Fuel Level: " << aircraft.GetFuelLevel() << "%\n";
        }
    }

    // Start listening for aircraft connections
    bool GroundTower::StartListening() {
        if (m_isListening) {
            std::cout << "Tower " << m_towerName << " is already listening.\n";
            return true;
        }

        // Initialize socket
        if (InitializeSocket() != SocketStatus::Success) {
            std::cerr << "Failed to initialize Winsock.\n";
            return false;
        }

        // Create socket
        m_server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_server_fd == INVALID_SOCKET) {
            std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
            CleanupSocket();
            return false;
        }

        // Start listening on a separate thread
        m_isListening = true;
        m_listenerThread = std::make_unique<std::thread>(&GroundTower::listenForAircraft, this);

        return true;
    }

    // Stop listening for aircraft connections
    void GroundTower::StopListening() {
        if (!m_isListening) {
            return;
        }

        m_isListening = false;

        if (m_server_fd != INVALID_SOCKET) {
            closesocket(m_server_fd);
            m_server_fd = INVALID_SOCKET;
        }

        if (m_listenerThread && m_listenerThread->joinable()) {
            m_listenerThread->join();
            m_listenerThread.reset();
        }

        CleanupSocket();
    }

    // Check if the tower is currently listening
    bool GroundTower::IsListening() const {
        return m_isListening;
    }

    // Listen for new aircraft connections
    void GroundTower::listenForAircraft() {
        struct sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(8080); // Port number

        // Bind socket
        if (bind(m_server_fd,
            reinterpret_cast<struct sockaddr*>(&serverAddress),
            sizeof(serverAddress)) == SOCKET_ERROR) {
            std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
            closesocket(m_server_fd);
            m_server_fd = INVALID_SOCKET;
            m_isListening = false;
            return;
        }

        // Listen for incoming connections
        if (listen(m_server_fd, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Listen failed: " << WSAGetLastError() << "\n";
            closesocket(m_server_fd);
            m_server_fd = INVALID_SOCKET;
            m_isListening = false;
            return;
        }

        std::cout << "Tower " << m_towerName << " is now listening for aircraft...\n";

		// Continuously accept new aircraft connections until stopped
        while (m_isListening) {
            struct sockaddr_in clientAddress;
            int clientAddressSize = sizeof(clientAddress);

            SOCKET clientSocket = accept(m_server_fd, reinterpret_cast<struct sockaddr*>(&clientAddress),&clientAddressSize);

            if (clientSocket == INVALID_SOCKET) {
				if (m_isListening) { 
                    std::cerr << "Accept failed: " << WSAGetLastError() << "\n"; } // Error if still listening
                continue;
            }

            // Handle communication with the connected aircraft
            std::thread communicationThread(&GroundTower::handleAircraftCommunication, this, clientSocket);
            communicationThread.detach();
        }
    }

    // Handle the data transmissions
    // Each time it receives the data from the aircraft (client) 
	// it will parse the data and update the aircraft information or register
    // if it is a new aircraft 
    void GroundTower::handleAircraftCommunication(SOCKET clientSocket) {
		const size_t bufferSize = 1024;     // Buffer size for incoming data
        char buffer[bufferSize] = { 0 };

        while (m_isListening) {
            // Clear buffer before receiving
            std::memset(buffer, 0, bufferSize);

            // Receive data from aircraft
            int bytesReceived = recv(clientSocket, buffer, bufferSize - 1, 0);

            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0'; // Null-terminate the received data
                std::cout << "Received from aircraft: " << buffer << "\n";

                // Parse message
                std::string message(buffer);
                Aircraft aircraft = parseAircraftMessage(message);

                // Check if the aircraft is already registered
                bool aircraftFound = false;
                {
                    std::lock_guard<std::mutex> lock(m_aircraftMutex);

                    for (const auto& existingAircraft : m_aircraftList) {
                        if (existingAircraft.GetAircraftID() == aircraft.GetAircraftID()) {
                            aircraftFound = true;
                            break;
                        }
                    }
                }

                // Register or update aircraft if already registered
                if (!aircraftFound) {
                    RegisterAircraft(aircraft);
                }
                else {
                    UpdateAircraft(aircraft);
                }

                // Send response to the aircraft
                std::string response = "Message received by " + m_towerName + " ground tower!";
                if (send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0) == SOCKET_ERROR) {
                    std::cerr << "Error sending response: " << WSAGetLastError() << "\n";
                    break;
                }
            }
            else if (bytesReceived == 0) {
                // Connection closed by the client
                std::cout << "Connection closed by aircraft.\n";
                break;
            }
            else {
                // Error in receiving data
                std::cerr << "Error receiving data: " << WSAGetLastError() << "\n";
                break;
            }
        }

        closesocket(clientSocket);
    }

    // Parse a message from an aircraft
    Aircraft GroundTower::parseAircraftMessage(const std::string& message) {
        std::istringstream messageStream(message);
		std::string token; // we use it to temporarly store the parsed data

        // Parse message format: 
        // ID,
        // latitude,
        // longitude,
        // altitude,
        // speed,
        // fuel
        std::string aircraftID;
        double latitude = 0.0;
        double longitude = 0.0;
        double altitude = 0.0;
        double speed = 0.0;
        double fuelLevel = 0.0;

        // ID
        if (std::getline(messageStream, token, ',')) {
            aircraftID = token;
        }

        // Latitude
        if (std::getline(messageStream, token, ',')) {
            try {
                latitude = std::stod(token);
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing latitude: " << e.what() << "\n";
            }
        }

        // Longitude
        if (std::getline(messageStream, token, ',')) {
            try {
                longitude = std::stod(token);
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing longitude: " << e.what() << "\n";
            }
        }

        // Altitude
        if (std::getline(messageStream, token, ',')) {
            try {
                altitude = std::stod(token);
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing altitude: " << e.what() << "\n";
            }
        }

        // Speed
        if (std::getline(messageStream, token, ',')) {
            try {
                speed = std::stod(token);
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing speed: " << e.what() << "\n";
            }
        }

        // Fuel level
        if (std::getline(messageStream, token)) {
            try {
                fuelLevel = std::stod(token);
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing fuel level: " << e.what() << "\n";
            }
        }

        return Aircraft(aircraftID, latitude, longitude, altitude, speed, fuelLevel);
    }
}