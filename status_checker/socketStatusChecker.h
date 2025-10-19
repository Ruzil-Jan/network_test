#pragma once

namespace Network {

    class SocketStatusChecker {
    public:
        static bool isPortBusy(int sockfd);
    };

}

