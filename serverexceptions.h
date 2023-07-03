#include <string>
#include <sstream>
#include <stdexcept>

#ifndef SERVEREXCEPTIONS_H
#define SERVEREXCEPTIONS_H

/**
 * @file    serverexceptions.h
 * @brief   File contains definitions of exceptions being thrown by ServerListener
 */

/**
 * @brief Parent class for all exceptions being thrown by the ServerListener
 */
class ServerException : public std::runtime_error {
public:
    /**
     * @param info Information about the exception (possibly shown to the user)
     */
    ServerException(char const *info) : std::runtime_error(info) {}

protected:
    ServerException(std::ostream const &info): std::runtime_error(static_cast<std::stringstream const &>(info).str()) {}
    static std::ostream &stream(std::stringstream const &s = std::stringstream()) { return const_cast<std::stringstream &>(s); }
};

/**
 * @brief Exception being thrown in case of socket library initialization error
 */
class ServerStartupException : public ServerException {
public:
    ServerStartupException()
        : ServerException("Socket library initialization failed") {}
};

/**
 * @brief Exception being thrown in case of addrinfo() returning an error code
 */
class AddrinfoException : public ServerException {
public:
    AddrinfoException(int error_no)
        : ServerException(
              stream() << "addrinfo() failed with error: " << error_no
        ) {}
};

/**
 * @brief Exception being thrown in case of socket() returning an error code
 */
class SocketCreationException : public ServerException {
public:
    SocketCreationException(int error_no)
        : ServerException(
              stream() << "socket() failed with error: " << error_no
        ) {}
};

/**
 * @brief Exception being thrown in case of bind() returning an error code
 */
class SocketBindingException : public ServerException {
public:
    SocketBindingException(int error_no)
        : ServerException(
              stream() << "bind() failed with error: " << error_no
        ) {}
};

/**
 * @brief Exception being thrown in case of listen() returning an error code
 */
class ListenException : public ServerException {
public:
    ListenException(int error_no)
        : ServerException(
              stream() << "listen() failed with error: " << error_no
        ) {}
};

/**
 * @brief Exception being thrown in case of accept() returning an error code
 */
class ClientAcceptationException : public ServerException {
public:
    ClientAcceptationException(int error_no)
        : ServerException(
              stream() << "accept() failed with error: " << error_no
        ) {}
};

#endif // SERVEREXCEPTIONS_H
