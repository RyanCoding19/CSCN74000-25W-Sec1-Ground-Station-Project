#pragma once
// ServerHeader.h - socket header for network communication
#ifndef SERVER_HEADER_H
#define SERVER_HEADER_H

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace PlaneSystem {

    /**
     * @brief Socket status error codes
     */
    enum class SocketStatus {
        Success = 0,
        WsaStartupFailed,
        SocketCreationFailed,
        BindFailed,
        ListenFailed,
        AcceptFailed,
        ConnectFailed,
        SendFailed,
        ReceiveFailed
    };

    /**
     * @brief Initialize socket
     * @return Status code indicating success or failure
     */
    SocketStatus InitializeSocket();

    /**
     * @brief Cleanup socket resources
     */
    void CleanupSocket();

}

#endif  // SERVER_HEADER_H