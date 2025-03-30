// GroundTower.h - ground control tower header file
#ifndef GROUNDTOWER_H
#define GROUNDTOWER_H

#include <string>
#include <vector>
#include <thread>
#include <winsock2.h> 
#include "Aircraft.h"
#include <iostream>
#include <cstring> 
#include <mutex>
#include <atomic>
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace PlaneSystem {

    /**
    * @class GroundTower
	* @brief Represents a ground control tower that communicates with aircraft (client)
    */
    class GroundTower {
    private:
        std::string m_towerName;                        /**< Tower's name */
        double m_latitude;                              /**< Tower's latitude */
        double m_longitude;                             /**< Tower's longitude */
        double m_operationalRadius;                     /**< Tower's operational radius in kilometers */

        mutable std::mutex m_aircraftMutex;             /**< Mutex to protect the aircraft list */
        std::vector<Aircraft> m_aircraftList;           /**< List of aircrafts connected to the tower */
        SOCKET m_server_fd;                             /**< Server Socket for accepting connections */

		std::atomic<bool> m_isListening;                /**< Flag to indicate if the tower is listening for aircraft */
		std::unique_ptr<std::thread> m_listenerThread;  /**< Thread to listen for aircraft connections */

        /**
		* @brief Listen for new aircraft (client) connections 
        */
        void listenForAircraft();

        /**
        * @breif Handle communication with a connected aircraft (client) 
		* @param clientSocket Socket for the connected aircraft (client)
        */
        void handleAircraftCommunication(SOCKET clientSocket);

        /**
        * @brief Parse a message from the connected aircraft (client)
        * @param message Message to parse
        * @return Aircraft object with the parsed data
        */
		Aircraft parseAircraftMessage(const std::string& message);

    public:
        /**
		* @brief Constructor to initialize the ground control tower
		* @param name Tower's name
		* @param lat Tower's latitude
		* @param lon Tower's longitude
		* @param radius Tower's operational radius in kilometers
        */
        GroundTower(const std::string& name, double lat, double lon, double radius);

        /**
		* @brief Destructor to clean up the allocated resources
        */
        ~GroundTower();

		// Delete opy constructor and assignment operator 
		GroundTower(const GroundTower&) = delete;
		GroundTower& operator=(const GroundTower&) = delete;

        /**
        * @brief Register an aircraft
		* @param aircraft Aircraft object to register
        */
        void RegisterAircraft(const Aircraft& aircraft);

        /**
        * @brief Update the aircraft location and metrics
		* @param aircraft Aircraft object to update
        */
        void UpdateAircraft(const Aircraft& aircraft);

        /**
        * @brief Display all registered aircraft
        */
        void DisplayAllAircraft() const;

        /**
		* @brief Start listening for aircraft connections
		* @return True if the server started successfully, false otherwise
        */
        bool StartListening();

        /**
		* @brief Stop listening for aircraft connections
        */
		void StopListening();

        /**
        * @brief Check if the tower is currently listening for aircraft connections
		* @return True if the tower is listening, false otherwise
        */
		bool IsListening() const;
    };
}

#endif // GROUNDTOWER_H
