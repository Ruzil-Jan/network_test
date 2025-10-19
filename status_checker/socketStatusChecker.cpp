#include "socketStatusChecker.h"
#include <unistd.h>
#include <sys/socket.h>

namespace Network {

    bool SocketStatusChecker::isPortBusy(int sockfd) {
        if (sockfd < 0) return false;

        // Try to use getsockopt to check if the socket is valid
        int error = 0;
        socklen_t len = sizeof(error);
        int result = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);

        return (result == 0 && error == 0);
    }

}

