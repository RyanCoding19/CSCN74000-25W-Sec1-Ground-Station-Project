// Aircraft.cpp - Implementation for aircraft (client)
#include "Aircraft.h"

namespace PlaneSystem {

    Aircraft::Aircraft(const std::string& id,
        double lat,
        double lon,
        double alt,
        double spd,
        double fuel) :
        m_aircraftID(id),
        m_latitude(lat),
        m_longitude(lon),
        m_altitude(alt),
        m_speed(spd),
        m_fuelLevel(fuel) {
    }

    Aircraft::Aircraft(const Aircraft& other)
        : m_aircraftID(other.m_aircraftID),
        m_latitude(other.m_latitude),
        m_longitude(other.m_longitude),
        m_altitude(other.m_altitude),
        m_speed(other.m_speed),
        m_fuelLevel(other.m_fuelLevel) {
    }

    Aircraft& Aircraft::operator=(const Aircraft& other) {
        if (this != &other) {
            m_aircraftID = other.m_aircraftID;
            m_latitude = other.m_latitude;
            m_longitude = other.m_longitude;
            m_altitude = other.m_altitude;
            m_speed = other.m_speed;
            m_fuelLevel = other.m_fuelLevel;
        }
        return *this;
    }

    // Getters implemetation
    std::string Aircraft::GetAircraftID() const { return m_aircraftID; }

    double Aircraft::GetLatitude() const { return m_latitude; }

    double Aircraft::GetLongitude() const { return m_longitude; }

    double Aircraft::GetAltitude() const { return m_altitude; }

    double Aircraft::GetSpeed() const { return m_speed; }

    double Aircraft::GetFuelLevel() const { return m_fuelLevel; }

    // Setters implementation
    void Aircraft::SetLatitude(double lat) { m_latitude = lat; }

    void Aircraft::SetLongitude(double lon) { m_longitude = lon; }

    void Aircraft::SetAltitude(double alt) { m_altitude = alt; }

    void Aircraft::SetSpeed(double spd) { m_speed = spd; }

    void Aircraft::SetFuelLevel(double fuel) { m_fuelLevel = fuel; }
}