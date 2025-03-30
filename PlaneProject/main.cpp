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

void handleCommandMenu(GroundTower& tower) {

    cout << "\n===== COMMAND CENTER =====\n";
    cout << "1. Send Weather Alert\n";
    cout << "2. Send Route Change Instruction\n";
    cout << "3. Send Altitude Change Instruction\n";
    cout << "4. Send Emergency Broadcast\n";
    cout << "5. Request System Diagnostic\n";
    cout << "0. Return to Main Menu\n";
    cout << "Enter choice: ";

    int choice;
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear input buffer

    if (choice < 0 || choice > 6) {
        cout << "Invalid option. Please try again.\n";
        return;
    }

    if (choice == 0) {
        return;
    }

    // Common variable for command handling
    string aircraftID;
    string customMessage;
    bool isBroadcast = false;
    bool success = false;
    int cmdSent = 0;
    GroundTower::CommandType cmdType;

    // Set command type based on user choice
    switch (choice) {
    case 1: 
        cmdType = GroundTower::CommandType::WEATHER_ALERT;
        break;
    case 2: 
        cmdType = GroundTower::CommandType::ROUTE_CHANGE; 
        break;
    case 3: 
        cmdType = GroundTower::CommandType::ALTITUDE_CHANGE; 
        break;
    case 4: 
        cmdType = GroundTower::CommandType::EMERGENCY_BROADCAST; 
        break;
    case 5: 
        cmdType = GroundTower::CommandType::SYSTEM_DIAGNOSTIC; 
        break;
    default: 
        return;
    }

    // Ask if broadcast or specific aircraft
    cout << "Send to (1) all aircraft or (2) specific aircraft? Enter choice: ";
    int sendOption;
    cin >> sendOption;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (sendOption == 1) {
        isBroadcast = true;
    }
    else if (sendOption == 2) {
        cout << "Enter aircraft ID: ";
        getline(cin, aircraftID);
    }
    else {
        cout << "Invalid option. Command canceled.\n";
        return;
    }

    // Execute the command
    if (isBroadcast) {
        cmdSent = tower.BroadcastCommand(cmdType);
        if (cmdSent > 0) {
            cout << "Command successfully broadcast to " << cmdSent << " aircraft.\n";
        }
        else {
            cout << "No aircraft available to receive broadcast.\n";
        }
    }
    else {
        success = tower.SendCommand(aircraftID, cmdType);
        if (success) {
            cout << "Command successfully sent to " << aircraftID << ".\n";
        }
        else {
            cout << "Failed to send command. Aircraft may not be connected.\n";
        }
    }
}

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
    cout << "Press 'h' to display communication history." << endl;
    cout << "Press 'c' to send commands to aircraft." << endl;
    cout << "Enter 'q' to quit." << endl;

    string input;
    while (true) {
        getline(cin, input);

        if (input == "q" || input == "Q") {
            break;  // Break the loop == exit the program
        }
        else if (input == "h" || input == "H") {
            // Display communication history
            tower.DisplayCommunicationHistory();
        }
        else if (input == "c" || input == "C") {
            // Handle command menu
            handleCommandMenu(tower);
        }
        else {
            // Display all registered aircraft whenever user presses Enter
            tower.DisplayAllAircraft();
        }

        cout << "\nPress Enter to refresh aircraft status," << endl;
        cout << "Press 'h' to display communication history," << endl;
        cout << "Press 'c' to send commands to aircraft," << endl;
        cout << "or 'q' to quit." << endl;
    }

    // Properly shut down the server
    tower.StopListening();
    cout << "Tower has stopped listening. Exiting program." << endl;

    return 0;
}
