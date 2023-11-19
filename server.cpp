#include <iostream>
#include <signal.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

constexpr char SOCKET_PATH[] = "/tmp/my_socket";

int server_socket;
int client_socket;

void handle_signal(int signal) {
  std::cout << "Received signal " << signal << ". Cleaning up and exiting..."
            << std::endl;

  // Close sockets
  close(client_socket);
  close(server_socket);

  // Remove the socket file
  unlink(SOCKET_PATH);

  exit(EXIT_SUCCESS);
}

int main() {

  socklen_t client_addr_len;
  sockaddr_un server_addr, client_addr;

  server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_socket == -1) {
    std::cout << "Error creating socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  server_addr.sun_family = AF_UNIX;
  strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

  signal(SIGINT, handle_signal);
  if (bind(server_socket, reinterpret_cast<struct sockaddr *>(&server_addr),
           sizeof(server_addr)) == -1) {
    std::cout << "Error binding socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (listen(server_socket, 5) == -1) {
    std::cout<<"Error listening for connections" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::cout << "Server is listening for connections...\n";

  client_addr_len = sizeof(client_addr);
  client_socket =
      accept(server_socket, reinterpret_cast<struct sockaddr *>(&client_addr),
             &client_addr_len);
  if (client_socket == -1) {
    std::cout << "Error accepting connection" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::cout << "Connection established.\n";

  char buffer[256];
  ssize_t bytes_received;

  while (true) {
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytes_received > 0) {
      std::cout << "Received command: " << std::string(buffer, bytes_received)
                << std::endl;
    } else if (bytes_received == 0) {
      std::cout << "Client disconnected. Waiting for a new connection..."
                << std::endl;
      close(client_socket);

      client_socket = accept(server_socket,
                             reinterpret_cast<struct sockaddr *>(&client_addr),
                             &client_addr_len);
      if (client_socket == -1) {
        perror("Error accepting connection");
        exit(EXIT_FAILURE);
      }

      std::cout << "Connection established." << std::endl;
    } else {
      std::cout << "Error receiving data" << std::endl;
      break; // Exit the loop on error
    }
  }

  return 0;
}
