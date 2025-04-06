// Aircraft.h
// Contains the aircrtaft (client) data structure
#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <string>

namespace PlaneSystem {

    /**
    * @class Aircraft
    * @brief Represents an aircraft with identification and telemetry data 
    */
    class Aircraft {
    private:
        std::string m_aircraftID; /**< Unique identifier for the aircraft */
        double m_latitude;        /**< Aircraft's latitude */
        double m_longitude;       /**< Aircraft's longitude */
        double m_altitude;        /**< Aircraft's altitude (meters) */
        double m_speed;           /**< Aircraft's speed (km/h) */
        double m_fuelLevel;       /**< Aircraft's fuel level (percentage) */

    public:
        /*
		* @brief Constructor to initialize aircraft data
        * @param id Unique identifier for the aircraft
        * @param lat Aircraft's latitude
        * @param lon Aircraft's longitude
        * @param alt Aircraft's altitude (meters)
        * @param spd Aircraft's speed (km/h)
        * @param fuel Aircraft's fuel level (percentage)
        */

        Aircraft()
            : m_aircraftID("UNKNOWN"), m_latitude(0.0), m_longitude(0.0),
            m_altitude(0.0), m_speed(0.0), m_fuelLevel(0.0) {
        }

        Aircraft(
            const std::string& id,
            double lat,
            double lon,
            double alt,
            double spd,
            double fuel
        );

        // Copy constructor
        Aircraft(const Aircraft& other);

		Aircraft& operator=(const Aircraft& other);

        // Getters 
        std::string GetAircraftID() const;
        double GetLatitude() const;
        double GetLongitude() const;
        double GetAltitude() const;
        double GetSpeed() const;
        double GetFuelLevel() const;

        // Setters
        void SetLatitude(double lat);
        void SetLongitude(double lon);
        void SetAltitude(double alt);
        void SetSpeed(double spd);
        void SetFuelLevel(double fuel);
    };
}
#endif // AIRCRAFT_H
