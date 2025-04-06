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

namespace PlaneSystem {

void handleCommandMenu(GroundTower& tower) {
    bool commandCanceled = false;
    int choice = -1;

    std::cout << "\n===== COMMAND CENTER =====\n";
    std::cout << "1. Send Weather Alert\n";
    std::cout << "2. Send Route Change Instruction\n";
    std::cout << "3. Send Altitude Change Instruction\n";
    std::cout << "4. Send Emergency Broadcast\n";
    std::cout << "5. Request System Diagnostic\n";
    std::cout << "0. Return to Main Menu\n";
    std::cout << "Enter choice: ";

    std::cin >> choice;
    (void)std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear input buffer

    if (choice < 0 || choice > 6) {
        std::cout << "Invalid option. Please try again.\n";
        commandCanceled = true;
    } else if (choice == 0) {
        // User chose to return to the main menu
        commandCanceled = true;
    }
    else {
        // Empty else statement
    }

    // Define variables needed for the rest of the function
    GroundTower::CommandType cmdType;
    bool isBroadcast = false;
    std::string aircraftID;
    
    // Set command type based on user choice if not canceled
    if (!commandCanceled) {
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
                // Should never reach here due to earlier check
                commandCanceled = true;
                break;
        }
    }

   
    if (!commandCanceled) {
        int sendOption = 0;
        std::cout << "Send to (1) all aircraft or (2) specific aircraft? Enter choice: ";
        std::cin >> sendOption;
        (void)std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (sendOption == 1) {
            isBroadcast = true;
        } else if (sendOption == 2) {
            std::cout << "Enter aircraft ID: ";
            (void)std::getline(std::cin, aircraftID);
        } else {
            std::cout << "Invalid option. Command canceled.\n";
            commandCanceled = true;
        }
    }

    // Execute the command if not canceled
    if (!commandCanceled) {
        if (isBroadcast) {
            int cmdSent = tower.BroadcastCommand(cmdType);
            if (cmdSent > 0) {
                std::cout << "Command successfully broadcast to " << cmdSent << " aircraft.\n";
            } else {
                std::cout << "No aircraft available to receive broadcast.\n";
            }
        } else {
            bool success = tower.SendCommand(aircraftID, cmdType);
            if (success) {
                std::cout << "Command successfully sent to " << aircraftID << ".\n";
            } else {
                std::cout << "Failed to send command. Aircraft may not be connected.\n";
            }
        }
    }

    return;
}

}
int main() {
    int exitCode = 0;  // Default to successful execution

    // Create GroundTower with name, latitude, longitude, and operational radius in kilometers
    PlaneSystem::GroundTower tower("Alpha Tower", 40.7128, -74.0060, 50.0);

    // Start the tower listening for aircraft
    if (!tower.StartListening()) {
        std::cerr << "Failed to start listening for aircraft.\n";
        exitCode = 1;
    }
    else {
        std::cout << "Tower is now listening for aircraft connections..." << std::endl;
        std::cout << "Press Enter to display current aircraft status." << std::endl;
        std::cout << "Press 'h' to display communication history." << std::endl;
        std::cout << "Press 'c' to send commands to aircraft." << std::endl;
        std::cout << "Enter 'q' to quit." << std::endl;

        std::string input;
        while (true) {
            (void)std::getline(std::cin, input);

            if (input == "q" || input == "Q") {
                break;  // Exit the loop
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
                // Display all registered aircraft whenever the user presses Enter
                tower.DisplayAllAircraft();
            }

            std::cout << "\nPress Enter to refresh aircraft status," << std::endl;
            std::cout << "Press 'h' to display communication history," << std::endl;
            std::cout << "Press 'c' to send commands to aircraft," << std::endl;
            std::cout << "or 'q' to quit." << std::endl;
        }

        // Properly shut down the server
        tower.StopListening();
        std::cout << "Tower has stopped listening. Exiting program." << std::endl;
    }

    return exitCode;
}
