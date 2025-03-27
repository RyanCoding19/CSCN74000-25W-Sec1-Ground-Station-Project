// Aircraft.h
#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <string>

using namespace std;

class Aircraft {
public:
    string aircraftID;  // Unique identifier for the aircraft
    double latitude; // Aircraft's latitude
    double longitude; // Aircraft's longitude
    double altitude;  // Aircraft's altitude (meters)
    double speed; // Aircraft's speed (km/h)
    double fuelLevel; // Aircraft's fuel level (percentage)

    // Constructor to initialize aircraft data
    Aircraft(const string& id, double lat, double lon, double alt, double spd, double fuel)
        : aircraftID(id), latitude(lat), longitude(lon), altitude(alt), speed(spd), fuelLevel(fuel) {}
};

#endif // AIRCRAFT_H
