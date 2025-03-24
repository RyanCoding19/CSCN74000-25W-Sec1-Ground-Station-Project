// main.cpp

// References:
// https://www.geeksforgeeks.org/socket-programming-in-cpp/
// https://github.com/seifzadeh/c-network-programming-best-snipts/blob/master/Code%20a%20simple%20socket%20client%20class%20in%20c%2B%2B
// https://www.w3schools.com/cpp/cpp_classes.asp
// https://www.linuxhowtos.org/C_C++/socket.htm
// https://www.geeksforgeeks.org/handling-multiple-clients-on-server-with-multithreading-using-socket-programming-in-c-cpp/
// https://www.geeksforgeeks.org/c-classes-and-objects/



#include "ServerHeader.h"
#include "GroundTower.h" 

using namespace std;

int main() {
    // Create GroundTower with name, latitude, longitude, and operational radius in kilometers
    GroundTower tower("Alpha Tower", 40.7128, -74.0060, 50.0);

    // Start the tower listening for aircraft
    tower.startListening();

    // Display all aircraft after registration
    tower.displayAllAircraft();

    // Simulate server running
    this_thread::sleep_for(chrono::minutes(10));

    return 0;
}