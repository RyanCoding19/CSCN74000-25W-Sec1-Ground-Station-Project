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
            (void)closesocket(m_server_fd);
            m_server_fd = INVALID_SOCKET;
        }

        if (m_listenerThread && m_listenerThread->joinable()) {
            m_listenerThread->join();
            m_listenerThread.reset();
        }

        CleanupSocket();
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
            (void)closesocket(m_server_fd);
            m_server_fd = INVALID_SOCKET;
            m_isListening = false;
            return;
        }

        // Listen for incoming connections
        if (listen(m_server_fd, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Listen failed: " << WSAGetLastError() << "\n";
            (void)closesocket(m_server_fd);
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
        std::string aircraftID;
        
        std::string greetingMessage = " TOWER Name: " + m_towerName + " | Latitude: " + std::to_string(m_latitude) + " | Longitude: " + std::to_string(m_longitude);
        if (send(clientSocket, greetingMessage.c_str(), static_cast<int>(greetingMessage.length()), 0) == SOCKET_ERROR) {
            std::cerr << "Error sending the greetings message: " << WSAGetLastError() << "\n";
        }

        while (m_isListening) {
            // Clear buffer before receiving
            (void)std::memset(buffer, 0, bufferSize);

            // Receive data from aircraft
            int bytesReceived = recv(clientSocket, buffer, bufferSize - 1, 0);

            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0'; // Null-terminate the received data
                std::string receivedMsg(buffer);
                std::cout << "Received from aircraft: " << buffer << "\n";

                // Parse message
                std::string message(buffer);
                Aircraft aircraft = parseAircraftMessage(message);

                // Store aircraft ID if this is our first message
                if (aircraftID.empty()) {
                    aircraftID = aircraft.GetAircraftID();

                    // Add to socket map 
                    {
                        std::lock_guard<std::mutex> lock(m_aircraftMutex);
                        m_aircraftSockets[aircraftID] = clientSocket;
                    }
                }

                // Log the incoming communication
                logCommunication(aircraft.GetAircraftID(), receivedMsg, true);

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

                // Log the outgoing communication
                logCommunication(aircraft.GetAircraftID(), response, false);
            }
            else if (bytesReceived == 0) {
                // Connection closed by the client
                std::cout << "Connection closed by aircraft " << aircraftID << ".\n";

                // Remove from socket map
                if (!aircraftID.empty()) {
                    std::lock_guard<std::mutex> lock(m_aircraftMutex);
                    (void)m_aircraftSockets.erase(aircraftID);
                }

                break;
            }
            else {
                // Error in receiving data
                std::cerr << "Error receiving data: " << WSAGetLastError() << "\n";

                // Remove from socket map
                if (!aircraftID.empty()) {
                    std::lock_guard<std::mutex> lock(m_aircraftMutex);
                    (void)m_aircraftSockets.erase(aircraftID);
                }

                break;
            }
        }

        (void)closesocket(clientSocket);
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

    // Get current timestamp
    std::string GroundTower::getCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);

        std::tm now_tm;
        (void)localtime_s(&now_tm, &now_c);

        std::stringstream ss;
        ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    // Check if the tower is currently listening
    bool GroundTower::IsListening() const {
        return m_isListening;
    }

    // Log communication
    void GroundTower::logCommunication(const std::string& aircraftID, const std::string& message, bool isIncoming) {
        std::lock_guard<std::mutex> lock(m_aircraftMutex);

        CommunicationLogEntry entry;
        entry.timestamp = getCurrentTimestamp();
        entry.aircraftID = aircraftID;
        entry.message = message;
        entry.isIncoming = isIncoming;

        m_communicationLog.push_back(entry);
    }

    // Method to display communication history
    void GroundTower::DisplayCommunicationHistory() const {
        std::lock_guard<std::mutex> lock(m_aircraftMutex);

        std::cout << "\n=== Communication History for " << m_towerName << " ===\n";
        if (m_communicationLog.empty()) {
            std::cout << "No communications recorded.\n";
            return;
        }

        for (const auto& entry : m_communicationLog) {
            std::cout << entry.timestamp << " | " << entry.aircraftID << " | " << (entry.isIncoming ? "RECEIVED" : "SENT") << " | ";
            std::cout << entry.message << "\n";
        }
    }

	// Send a command to an aircraft
    bool GroundTower::SendCommand(const std::string& aircraftID, CommandType commandType) {
        SOCKET clientSocket;
        bool aircraftExists = false;

        // First get the client socket while holding the lock
        {
            std::lock_guard<std::mutex> lock(m_aircraftMutex);
            auto it = m_aircraftSockets.find(aircraftID);
            if (it != m_aircraftSockets.end()) {
                clientSocket = it->second;
                aircraftExists = true;
            }
        }

        if (!aircraftExists) {
            std::cerr << "Cannot send command: Aircraft " << aircraftID << " not connected.\n";
            return false;
        }

        // Format command message based on type
        std::string commandMessage = "CMD:";
        std::string timestamp = getCurrentTimestamp();

        // Switch statement to determine the type of command
        switch (commandType) {
        case CommandType::WEATHER_ALERT:
            commandMessage += "[WEATHER ALERT] " + timestamp + " - Severe weather conditions ahead. "
                + "Prepare for turbulence and possible route changes.";
            break;
        case CommandType::ROUTE_CHANGE:
            commandMessage += "[ROUTE CHANGE] " + timestamp + " - Please prepare for route adjustment. "
                + "New navigation coordinates will follow.";
            break;
        case CommandType::ALTITUDE_CHANGE:
            commandMessage += "[ALTITUDE CHANGE] " + timestamp + " - Altitude adjustment required. "
                + "Please standby for new altitude instructions.";
            break;
        case CommandType::EMERGENCY_BROADCAST:
            commandMessage += "[EMERGENCY ALERT] " + timestamp + " - ATTENTION: Emergency situation reported. "
                + "All aircraft maintain current vectors and standby for instructions.";
            break;
        case CommandType::SYSTEM_DIAGNOSTIC:
            commandMessage += "[SYSTEM DIAGNOSTIC] " + timestamp + " - Please perform standard system diagnostic check "
                + "and report back with results.";
            break;
        default:
            std::cerr << "Unknown command type.\n";
            return false;
        }

        // Send the command
        if (send(clientSocket, commandMessage.c_str(), static_cast<int>(commandMessage.length()), 0) == SOCKET_ERROR) {
            std::cerr << "Error sending command to " << aircraftID << ": " << WSAGetLastError() << "\n";
            return false;
        }

        // Log the outgoing command
        logCommunication(aircraftID, commandMessage, false);
        std::cout << "Command sent to " << aircraftID << ": " << commandMessage << "\n";

        return true;
    }

	// Broadcast a command to all connected aircraft
    int GroundTower::BroadcastCommand(CommandType commandType) {
        std::vector<std::string> aircraftIDs;

        {
            std::lock_guard<std::mutex> lock(m_aircraftMutex);
            if (m_aircraftSockets.empty()) {
                std::cout << "No aircraft connected to broadcast command.\n";
                return 0;
            }

            // Copy the aircraft IDs
            for (const auto& pair : m_aircraftSockets) {
                aircraftIDs.push_back(pair.first);
            }
        }

        int successCount = 0;

        // Send the command to each aircraft using their IDs
        for (const auto& id : aircraftIDs) {
            if (SendCommand(id, commandType)) {
                successCount++;
            }
        }

        std::cout << "Command broadcast to " << successCount << " aircraft.\n";
        return successCount;
    }
}