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
using namespace PlaneSystem;

int main() {
    // Create GroundTower with name, latitude, longitude, and operational radius in kilometers
    GroundTower tower("Alpha Tower", 40.7128, -74.0060, 50.0);

    // Start the tower listening for aircraft
	if (!tower.StartListening()) {
		cerr << "Failed to start listening for aircraft.\n";
		return 1;
	}

    cout << "Tower is now listening for aircraft connections..." << endl;
    cout << "Press Enter to display current aircraft status." << endl;
    cout << "Enter 'q' to quit." << endl;

    string input;
    while (true) {
        getline(cin, input);

        if (input == "q" || input == "Q") {
            break;  // Break the loop == exit the program
        }

        // Display all registered aircraft whenever user presses Enter
        tower.DisplayAllAircraft();

        cout << "Press Enter to refresh aircraft status or 'q' to quit." << endl;
    }

    // Properly shut down the server
    tower.StopListening();
    cout << "Tower has stopped listening. Exiting program." << endl;

    return 0;
}