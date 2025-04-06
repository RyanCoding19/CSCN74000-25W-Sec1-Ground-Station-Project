// ServerHeader.cpp - Implementation for server header 
#include "ServerHeader.h"

namespace PlaneSystem {

    SocketStatus InitializeSocket() {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        SocketStatus status = SocketStatus::Success;

        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            status = SocketStatus::WsaStartupFailed;
        }
        return status;
    }

    void CleanupSocket() {
        WSACleanup();
    }
} 