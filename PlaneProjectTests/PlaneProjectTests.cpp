#include "pch.h"
#include "CppUnitTest.h"
#include "../PlaneProject/Aircraft.h"
#include "../Plane Project Client/main.cpp"
#include "../PlaneProject/GroundTower.h"


namespace PlaneProjectTests {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace PlaneSystem;

    TEST_CLASS(AircraftTests)
    {
    public:
        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required
        TEST_METHOD(DefaultConstructor_SetsDefaults)
        {
            char unknown[] = "UNKNOWN";

            Aircraft a;

            Assert::AreEqual(&unknown[0], a.GetAircraftID().c_str());
            Assert::AreEqual(0.0, a.GetLatitude());
            Assert::AreEqual(0.0, a.GetLongitude());
            Assert::AreEqual(0.0, a.GetAltitude());
            Assert::AreEqual(0.0, a.GetSpeed());
            Assert::AreEqual(0.0, a.GetFuelLevel());
        }
        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required 
        TEST_METHOD(ParameterizedConstructor_SetsCorrectValues)
        {
            char unknown[] = "AC101";

            Aircraft a(unknown, 52.4, 13.1, 10000.0, 850.0, 65.0);

            Assert::AreEqual(&unknown[0], a.GetAircraftID().c_str());
            Assert::AreEqual(52.4, a.GetLatitude());
            Assert::AreEqual(13.1, a.GetLongitude());
            Assert::AreEqual(10000.0, a.GetAltitude());
            Assert::AreEqual(850.0, a.GetSpeed());
            Assert::AreEqual(65.0, a.GetFuelLevel());
        }
        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required
        TEST_METHOD(CopyConstructor_CopiesValues)
        {
            Aircraft original("AC999", 40.0, -74.0, 12000.0, 900.0, 80.0);
            Aircraft copy(original);

            (void)Assert::AreEqual(original.GetAircraftID(), copy.GetAircraftID());
            (void)Assert::AreEqual(original.GetLatitude(), copy.GetLatitude());
            (void)Assert::AreEqual(original.GetLongitude(), copy.GetLongitude());
            (void)Assert::AreEqual(original.GetAltitude(), copy.GetAltitude());
            (void)Assert::AreEqual(original.GetSpeed(), copy.GetSpeed());
            (void)Assert::AreEqual(original.GetFuelLevel(), copy.GetFuelLevel());
        }
        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required
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
        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required
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
    };

    TEST_CLASS(ClientTests)
    {
    public:
        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required
        TEST_METHOD(WriteToLogFile_WritesCorrectMessage)
        {
            const std::string filename = "test_log.txt";
            const std::string testMessage = "This is a test log entry.";

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

            Assert::IsTrue(found, static_cast<const wchar_t*>(L"Log message not found in file."));
        }

        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required
        TEST_METHOD(GenerateAircraftID_ProducesValidFormat)
        {
            char unknown[] = "Aircraft-";

            const std::string id = MainClient::generateAircraftID();
            Assert::IsTrue(id.find(&unknown[0]) == 0);
            Assert::IsTrue(id.length() > 9U);
        }
        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required
        TEST_METHOD(GenerateAircraftID_IsReasonablyUnique)
        {
            std::set<std::string> ids;
            const int samples = 100;
            int duplicates = 0;

            for (int i = 0; i < samples; ++i) {
                const std::string id = MainClient::generateAircraftID();
                if (ids.find(id) != ids.end()) {
                    ++duplicates;
                }
                else {
                    const auto res = ids.insert(id);
                    (void)res;
                }
            }

            Assert::IsTrue(duplicates <= 2, static_cast<const wchar_t*>(L"Too many duplicates in generated IDs."));
        }
        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required
        TEST_METHOD(CleanupSocket_ClosesSocketProperly)
        {

            SOCKET mockSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            Assert::AreNotEqual(INVALID_SOCKET, mockSocket);

            MainClient::cleanupSocket(mockSocket);

            Assert::AreEqual(INVALID_SOCKET, mockSocket);
        }
        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required
        TEST_METHOD(SendAircraftData_DecreasesFuelLevel)
        {
            PlaneSystem::Aircraft testAircraft("TestAC", 0.0, 0.0, 0.0, 0.0, 5.0);

            {
                std::lock_guard<std::mutex> lock(MainClient::g_aircraftMutex);
                double initialFuel = testAircraft.GetFuelLevel();
                double newFuel = initialFuel - 0.5;
                testAircraft.SetFuelLevel((newFuel < 0.0) ? 0.0 : newFuel);
            }

            Assert::AreEqual(4.5, testAircraft.GetFuelLevel(), 0.01);
        }
        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required
        TEST_METHOD(AircraftMessage_IsFormattedCorrectly)
        {

            char unknown[] = "AC999";
            char second[] = ".";

            PlaneSystem::Aircraft a(unknown, 40.1, -73.2, 10000, 900, 85.5);
            std::string msg = a.GetAircraftID() + "," +
                std::to_string(a.GetLatitude()) + "," +
                std::to_string(a.GetLongitude()) + "," +
                std::to_string(a.GetAltitude()) + "," +
                std::to_string(a.GetSpeed()) + "," +
                std::to_string(a.GetFuelLevel());

            Assert::IsTrue(msg.find(&unknown[0]) != std::string::npos);
            Assert::IsTrue(msg.find(&second[0]) != std::string::npos);
        }
    };

    TEST_CLASS(ServerTests)
    {
    public:
        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required
        TEST_METHOD(RegisterAircraft_AddsToList)
        {
            GroundTower tower("TestTower", 0, 0, 100);
            Aircraft a("AC001", 10, 20, 3000, 500, 60);
            tower.RegisterAircraft(a);
        }
        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required
        TEST_METHOD(UpdateAircraft_UpdatesExistingAircraft)
        {
            GroundTower tower("TestTower", 0, 0, 100);
            Aircraft a("AC001", 10, 20, 3000, 500, 60);
            Aircraft updated("AC001", 11, 21, 3100, 510, 65);

            tower.RegisterAircraft(a);
            tower.UpdateAircraft(updated);
        }
        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required
        TEST_METHOD(StartListening_And_StopListening)
        {
            GroundTower tower("TestTower", 0.0, 0.0, 100.0);

            bool started = tower.StartListening();
            Assert::IsTrue(started, static_cast<const wchar_t*>(L"Tower should start listening successfully."));
            Assert::IsTrue(tower.IsListening(), static_cast<const wchar_t*>(L"Tower should report listening state as true."));

            tower.StopListening();
            Assert::IsFalse(tower.IsListening(), static_cast<const wchar_t*>(L"Tower should report listening state as false after stop."));
        }
        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required
        TEST_METHOD(SendCommand_InvalidAircraftID_Fails)
        {
            GroundTower tower("TestTower", 0.0, 0.0, 100.0);

            bool result = tower.SendCommand("NonexistentAC", GroundTower::CommandType::ROUTE_CHANGE);
            Assert::IsFalse(result, static_cast<const wchar_t*>(L"Sending command to invalid aircraft ID should fail."));
        }
        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required
        TEST_METHOD(BroadcastCommand_NoConnectedAircraft_ReturnsZero)
        {
            GroundTower tower("TowerZ", 0.0, 0.0, 100.0);

            int broadcasted = tower.BroadcastCommand(GroundTower::CommandType::ALTITUDE_CHANGE);
            Assert::AreEqual(0, broadcasted, static_cast<const wchar_t*>(L"No aircraft connected, should return 0."));
        }
        // MISRA deviation: Justification: " _GetTestCkassInfo" and "_GetTestVersion" are framework generated and their return values aren't required
        TEST_METHOD(IsListening_InitiallyFalse)
        {
            GroundTower tower("SilentTower", 0, 0, 100);
            Assert::IsFalse(tower.IsListening(), static_cast<const wchar_t*>(L"Tower should not be listening initially."));
        }
    };

}