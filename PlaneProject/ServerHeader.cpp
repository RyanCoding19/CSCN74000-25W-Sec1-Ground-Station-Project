// ServerHeader.cpp - Implementation for server header 
#include "ServerHeader.h"

namespace PlaneSystem {

    SocketStatus InitializeSocket() {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            return SocketStatus::WsaStartupFailed;
        }
        return SocketStatus::Success;
    }

    void CleanupSocket() {
        WSACleanup();
    }
} 