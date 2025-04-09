#include "pch.h"  
#include "CppUnitTest.h"
#include "../PlaneProject/Aircraft.h"
#include "../Plane Project Client/main.cpp"
#include "../PlaneProject/GroundTower.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace PlaneSystem;

namespace PlaneProjectTests
{

        TEST_CLASS(AircraftTests)
        {
        public:

            TEST_METHOD(DefaultConstructor_SetsDefaults)
            {
                Aircraft a;

                Assert::AreEqual(std::string("UNKNOWN"), a.GetAircraftID());
                Assert::AreEqual(0.0, a.GetLatitude());
                Assert::AreEqual(0.0, a.GetLongitude());
                Assert::AreEqual(0.0, a.GetAltitude());
                Assert::AreEqual(0.0, a.GetSpeed());
                Assert::AreEqual(0.0, a.GetFuelLevel());
            }

            TEST_METHOD(ParameterizedConstructor_SetsCorrectValues)
            {
                Aircraft a("AC101", 52.4, 13.1, 10000.0, 850.0, 65.0);

                Assert::AreEqual(std::string("AC101"), a.GetAircraftID());
                Assert::AreEqual(52.4, a.GetLatitude());
                Assert::AreEqual(13.1, a.GetLongitude());
                Assert::AreEqual(10000.0, a.GetAltitude());
                Assert::AreEqual(850.0, a.GetSpeed());
                Assert::AreEqual(65.0, a.GetFuelLevel());
            }

            TEST_METHOD(CopyConstructor_CopiesValues)
            {
                Aircraft original("AC999", 40.0, -74.0, 12000.0, 900.0, 80.0);
                Aircraft copy(original);

                Assert::AreEqual(original.GetAircraftID(), copy.GetAircraftID());
                Assert::AreEqual(original.GetLatitude(), copy.GetLatitude());
                Assert::AreEqual(original.GetLongitude(), copy.GetLongitude());
                Assert::AreEqual(original.GetAltitude(), copy.GetAltitude());
                Assert::AreEqual(original.GetSpeed(), copy.GetSpeed());
                Assert::AreEqual(original.GetFuelLevel(), copy.GetFuelLevel());
            }

            TEST_METHOD(AssignmentOperator_CopiesValues)
            {
                Aircraft a("AC1", 10.0, 20.0, 3000.0, 400.0, 90.0);
                Aircraft b;
                b = a;

                Assert::AreEqual(a.GetAircraftID(), b.GetAircraftID());
                Assert::AreEqual(a.GetLatitude(), b.GetLatitude());
                Assert::AreEqual(a.GetLongitude(), b.GetLongitude());
                Assert::AreEqual(a.GetAltitude(), b.GetAltitude());
                Assert::AreEqual(a.GetSpeed(), b.GetSpeed());
                Assert::AreEqual(a.GetFuelLevel(), b.GetFuelLevel());
            }

            TEST_METHOD(Setters_UpdateFieldsCorrectly)
            {
                Aircraft a("Test", 0, 0, 0, 0, 0);

                a.SetLatitude(35.5);
                a.SetLongitude(-120.5);
                a.SetAltitude(9500.0);
                a.SetSpeed(780.0);
                a.SetFuelLevel(55.5);

                Assert::AreEqual(35.5, a.GetLatitude());
                Assert::AreEqual(-120.5, a.GetLongitude());
                Assert::AreEqual(9500.0, a.GetAltitude());
                Assert::AreEqual(780.0, a.GetSpeed());
                Assert::AreEqual(55.5, a.GetFuelLevel());
            }
            TEST_METHOD(AssignmentOperator_SelfAssignment)
            {
                Aircraft a("Selfie", 1, 2, 3, 4, 5);
                a = a;
                Assert::AreEqual(std::string("Selfie"), a.GetAircraftID());
                Assert::AreEqual(1.0, a.GetLatitude());
                Assert::AreEqual(2.0, a.GetLongitude());
                Assert::AreEqual(3.0, a.GetAltitude());
                Assert::AreEqual(4.0, a.GetSpeed());
                Assert::AreEqual(5.0, a.GetFuelLevel());
            }
        };
        TEST_CLASS(ClientTests)
        {
        public:

            TEST_METHOD(WriteToLogFile_WritesCorrectMessage)
            {
                std::string filename = "test_log.txt";
                std::string testMessage = "This is a test log entry.";

                // Clear file before writing
                std::ofstream clearFile(filename, std::ios::trunc);
                clearFile.close();

                MainClient::writeToLogFile(filename, testMessage);

                std::ifstream inFile(filename);
                Assert::IsTrue(inFile.is_open());

                std::string line;
                bool found = false;
                while (std::getline(inFile, line)) {
                    if (line.find(testMessage) != std::string::npos) {
                        found = true;
                        break;
                    }
                }

                Assert::IsTrue(found, L"Log message not found in file.");
            }

            TEST_METHOD(GenerateAircraftID_ProducesValidFormat)
            {
                std::string id = MainClient::generateAircraftID();
                Assert::IsTrue(id.find("Aircraft-") == 0);
                Assert::IsTrue(id.length() > 9); // e.g., Aircraft-1234
            }

            TEST_METHOD(GenerateAircraftID_IsReasonablyUnique)
            {
                std::set<std::string> ids;
                const int samples = 100;
                int duplicates = 0;

                for (int i = 0; i < samples; ++i) {
                    std::string id = MainClient::generateAircraftID();
                    if (ids.find(id) != ids.end()) {
                        duplicates++;
                    }
                    else {
                        ids.insert(id);
                    }
                }

                Assert::IsTrue(duplicates <= 2, L"Too many duplicates in generated IDs.");
            }

            TEST_METHOD(CleanupSocket_ClosesSocketProperly)
            {
                SOCKET mockSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                Assert::AreNotEqual(INVALID_SOCKET, mockSocket);

                MainClient::cleanupSocket(mockSocket);

                Assert::AreEqual(INVALID_SOCKET, mockSocket);
            }
        };
        TEST_CLASS(ServerTests)
        {
            TEST_METHOD(RegisterAircraft_AddsToList)
            {
                GroundTower tower("TestTower", 0, 0, 100);
                Aircraft a("AC001", 10, 20, 3000, 500, 60);
                tower.RegisterAircraft(a);
            }

            TEST_METHOD(UpdateAircraft_UpdatesExistingAircraft)
            {
                GroundTower tower("TestTower", 0, 0, 100);
                Aircraft a("AC001", 10, 20, 3000, 500, 60);
                Aircraft updated("AC001", 11, 21, 3100, 510, 65);

                tower.RegisterAircraft(a);
                tower.UpdateAircraft(updated);
            }

            TEST_METHOD(StartListening_And_StopListening)
            {
                GroundTower tower("TestTower", 0.0, 0.0, 100.0);

                bool started = tower.StartListening();
                Assert::IsTrue(started, L"Tower should start listening successfully.");
                Assert::IsTrue(tower.IsListening(), L"Tower should report listening state as true.");

                tower.StopListening();
                Assert::IsFalse(tower.IsListening(), L"Tower should report listening state as false after stop.");
            }

            TEST_METHOD(RegisterAndUpdateAircraft_WorksCorrectly)
            {
                GroundTower tower("TowerOne", 0, 0, 50.0);
                Aircraft a1("AC001", 10, 20, 3000, 400, 70);
                Aircraft a1Updated("AC001", 11, 21, 3100, 410, 65);

                tower.RegisterAircraft(a1);
                tower.UpdateAircraft(a1Updated);
            }

            TEST_METHOD(DisplayAllAircraft_DoesNotThrow)
            {
                GroundTower tower("TowerEcho", 0.0, 0.0, 100.0);
                Aircraft ac("ACXYZ", 1.1, 2.2, 1000, 300, 90);
                tower.RegisterAircraft(ac);

                tower.DisplayAllAircraft();
            }

            TEST_METHOD(SendCommand_InvalidAircraftID_Fails)
            {
                GroundTower tower("TestTower", 0.0, 0.0, 100.0);

                bool result = tower.SendCommand("NonexistentAC", GroundTower::CommandType::ROUTE_CHANGE);
                Assert::IsFalse(result, L"Sending command to invalid aircraft ID should fail.");
            }

            TEST_METHOD(BroadcastCommand_NoConnectedAircraft_ReturnsZero)
            {
                GroundTower tower("TowerZ", 0.0, 0.0, 100.0);

                int broadcasted = tower.BroadcastCommand(GroundTower::CommandType::ALTITUDE_CHANGE);
                Assert::AreEqual(0, broadcasted, L"No aircraft connected, should return 0.");
            }

            TEST_METHOD(StartListening_CalledTwice_IsSafe)
            {
                GroundTower tower("TowerIdempotent", 0, 0, 100);

                bool first = tower.StartListening();
                bool second = tower.StartListening();

                Assert::IsTrue(first, L"First StartListening should return true.");
                Assert::IsTrue(second, L"Second StartListening should return true (already listening).");

                tower.StopListening();
            }

            TEST_METHOD(IsListening_InitiallyFalse)
            {
                GroundTower tower("SilentTower", 0, 0, 100);
                Assert::IsFalse(tower.IsListening(), L"Tower should not be listening initially.");
            }

        };
}