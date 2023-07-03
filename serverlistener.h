#ifndef SERVERLISTENER_H
#define SERVERLISTENER_H

/**
 * @file    serverlistener.h
 * @brief   File contains definition of ServerListener
 */

#include <stdexcept>
#include <string>
#include <functional>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "serverexceptions.h"

/**
 * @brief Creates an HTTP server instance and awaits requests
 *
 * Class uses WinSock library.
 *
 * @warning Server handles only HTTP GET requests (and shows some debug data in response)
 * @warning Threads responsible for handling clients aren't really being taken care of
 * @todo Handle more HTTP methods
 * @todo Give the opportunity to handle client requests to the user (some kind of routing)
 */
class ServerListener {
    char const *port;
    static size_t const buffer_size = 255;
    SOCKET listen_socket;
    addrinfo *socket_props;
    bool server_running;

    static DWORD CALLBACK clientHandler(SOCKET client_socket);

public:

    /**
     * Initialize a new server instance.
     *
     * @param port Port on which the server will be listening for connections
     * @param buffer_size Size of the buffer used to retrieve data from sockets
     */
    ServerListener(char const *port);

    /**
      * Start listening for connections.
      *
      * @param client_acceptation_error_callback The function receiving the ClientAcceptationException object when a problem with acceptation of new connection occurs
      */
    void run();

    /**
     * Stop listening for connections (close listening socket).
     *
     * @warning It could be run only from the different thread, as start() is blocking
     */
    void stop();

    virtual ~ServerListener();
};

#endif // SERVERLISTENER_H
