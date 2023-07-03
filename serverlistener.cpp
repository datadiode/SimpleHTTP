#include "serverlistener.h"

#include <iostream>
#include <cstdlib>
#include <list>

#include "requestparser.h"

ServerListener::ServerListener(char const *port) {
    this->port = port;
    this->listen_socket = INVALID_SOCKET;
    this->socket_props = NULL;
    this->server_running = false;

    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw ServerStartupException();
    }
}

void ServerListener::run() {
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int addrinfo_status = getaddrinfo(NULL, port, &hints, &socket_props);
    if(addrinfo_status != 0) {
        throw AddrinfoException(addrinfo_status);
    }

    listen_socket = socket(socket_props->ai_family, socket_props->ai_socktype, socket_props->ai_protocol);
    if(listen_socket == INVALID_SOCKET) {
        throw SocketCreationException(WSAGetLastError());
    }

    if(bind(listen_socket, socket_props->ai_addr, (int)socket_props->ai_addrlen) == SOCKET_ERROR) {
        throw SocketBindingException(WSAGetLastError());
    }

    if(listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        throw ListenException(WSAGetLastError());
    }

    bool server_running = true;
    while(server_running) {
        SOCKET client_socket = accept(listen_socket, NULL, NULL);
        if(client_socket == INVALID_SOCKET) {
            continue;
        }
        if (HANDLE h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServerListener::clientHandler, (LPVOID)client_socket, 0, NULL)) {
            CloseHandle(h);
        }
    }
}

void ServerListener::stop() {
    server_running = false;
    if(listen_socket != INVALID_SOCKET) {
        shutdown(listen_socket, SD_BOTH);
        closesocket(listen_socket);
        listen_socket = INVALID_SOCKET;
    }
    if (socket_props) {
        freeaddrinfo(socket_props);
        socket_props = NULL;
    }
}

DWORD ServerListener::clientHandler(SOCKET client_socket) {
    char recvbuf[buffer_size];
    int bytes_received;
    RequestParser parser;

    sockaddr_in client_info;
    int client_info_len = sizeof(sockaddr_in);
    char *client_ip;

    if(getpeername(client_socket, (sockaddr*)(&client_info), &client_info_len) == SOCKET_ERROR) {
        goto cleanup;
    }
    client_ip = inet_ntoa(client_info.sin_addr);

    while(1) {
        parser.reset();

        bool headers_ready = false;
        while(!headers_ready) {
            bytes_received = recv(client_socket, recvbuf, buffer_size, 0);
            if(bytes_received > 0) {
                parser.processChunk(recvbuf, bytes_received);
                if(parser.allHeadersAvailable()) {
                    headers_ready = true;
                }
            } else {
                goto cleanup;
            }
        }

        std::map<std::string, std::string> headers = parser.getHeaders();

        std::map<std::string, std::string>::iterator conn_it = headers.find("Connection");
        if(conn_it != headers.end() && conn_it->second == "close") {
            goto cleanup;
        }

        std::cout << parser.getMethod() << " "
                  << parser.getPath() << " "
                  << parser.getProtocol() << "\n";

        std::cout << "> " << client_ip << "\n";

        std::map<std::string, std::string>::iterator ua_it = headers.find("User-Agent");
        if(ua_it != headers.end()) {
            std::cout << "> " << ua_it->second << "\n";
        } else {
            std::cout << "> no UAString provided" << "\n";
        }

        std::cout << "\n";

        std::stringstream response_body;

        if (parser.getMethod() == "GET") {
            response_body <<
                "<!DOCTYPE html>"
                "<title>Request info</title>"
                "<style>"
                "table, th, td { border: 1px solid #333; border-collapse: collapse; }"
                "th, td { padding: 3px 5px; }"
                "th { text-align: right; }"
                "td { text-align: left; }"
                "</style>"
                "<h1>JavaScript test</h1>"
                "<table>"
                "<tr><th>new Date().toTimeString()</th><td><script>document.write(new Date().toTimeString())</script></td></tr>"
                "<tr><th>'JavaScript'.toLowerCase()</th><td><script>document.write('JavaScript'.toLowerCase())</script></td></tr>"
                "<tr><th>'JavaScript'.toUpperCase()</th><td><script>document.write('JavaScript'.toUpperCase())</script></td></tr>"
                "</table>"
                "<h1>Request info</h1>";
        } else {
            response_body <<
                "<h1>XMLHttpRequest info</h1>";
        }

        response_body <<
            "<table>"
            "<tr><th>Method</th><td>" << parser.getMethod() << "</td></tr>"
            "<tr><th>Path</th><td>" << parser.getPath() << "</td></tr>"
            "<tr><th>Protocol</th><td>" << parser.getProtocol() << "</td></tr>";

        for(std::map<std::string, std::string>::iterator header = headers.begin(); header != headers.end(); ++header) {
            response_body <<
                "<tr><th>" << header->first << "</th><td>" << header->second << "</td></tr>";
        }

        response_body <<
            "<tr><th>Thread ID</th><td>" << GetCurrentThreadId() << "</td></tr>"
            "</table>";

        if (parser.getMethod() == "GET") {
            response_body <<
                "<script>\r\n"
                "var xmlreq = new XMLHttpRequest();\r\n"
                "xmlreq.open('POST', 'headerbugtest.php', true);\r\n"
                "xmlreq.setRequestHeader('Content-Type', 'application/json');\r\n"
                "xmlreq.onreadystatechange = function() {\r\n"
                    "if (xmlreq.readyState == 4) {\r\n"
                        "document.body.insertAdjacentHTML('beforeend', xmlreq.responseText);\r\n"
                    "}\r\n"
                "}\r\n"
                "xmlreq.send('{}');\r\n"
                "</script>\r\n";
        }

        response_body.seekg(0, std::ios::end);

        std::stringstream response_headers;
        response_headers <<
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "Connection: keep-alive\r\n"
            "Content-Length: " << response_body.tellg() << "\r\n\r\n";

        std::string response = response_headers.str() + response_body.str();
        send(client_socket, response.c_str(), static_cast<int>(response.length()), 0);
    }
cleanup:
    closesocket(client_socket);
    return 0;
}

ServerListener::~ServerListener() {
    stop();
    WSACleanup();
}
