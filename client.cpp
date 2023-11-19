#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

constexpr char SOCKET_PATH[] = "/tmp/my_socket";

int main() {

    int client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (connect(client_socket, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) == -1) {
        std::cout<< "Error connecting to server" << std::endl;
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    std::string command = "test";
    ssize_t bytes_sent = send(client_socket, command.c_str(), command.length(), 0);
    if (bytes_sent == -1) {
        std::cout <<"Error sending command" << std::endl;
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    close(client_socket);

    return 0;
}