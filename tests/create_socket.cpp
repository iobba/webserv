#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    // Create a socket using the socket() function
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        std::perror("Socket creation failed");
        return (EXIT_FAILURE);
    }

    std::cout << "Socket created successfully. Socket descriptor: " << socket_fd << std::endl;

     // IP address and port number
    const char* ipAddress = "127.0.0.1";
    int portNumber = 8080;

    // Create a sockaddr_in structure for binding
    struct sockaddr_in server_address;
    // initialize all bytes to ZERO to avoid the uninitialized padding bytes
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portNumber);
    // this way of filling addresses works only with IPv4
    server_address.sin_addr.s_addr = inet_addr(ipAddress);

    // Bind the socket to the specified address and port
    if (bind(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
    {
        std::perror("Bind failed");
        close(socket_fd);
        return (EXIT_FAILURE);
    }

    std::cout << "Socket bound successfully to IP: " << ipAddress << " and port: " << portNumber << std::endl;
    std::cout << "Socket descriptor: " << socket_fd << std::endl;

    // listen
    int backlog = 5;
    if (listen(socket_fd, backlog) == -1)
    {
        std::perror("Listen failed");
        close(socket_fd);
        return (EXIT_FAILURE);
    }

    struct sockaddr_in client_address;
    socklen_t client_addr_len = sizeof(client_address);
    int clients_count = 1;
    while (clients_count < 4)
    {
        // accept
        int client_socket_fd = accept(socket_fd, (struct sockaddr*)&client_address, &client_addr_len);
        if (client_socket_fd == -1)
        {
            std::perror("Accept failed");
            close(socket_fd);
            return (EXIT_FAILURE);
        }

        char buffer[1024] = {0};
        ssize_t valread = recv(client_socket_fd , buffer, 1024, 0); 
        printf("the recieved data is : | %s %d |\n",buffer , clients_count);
        if(valread < 0)
        { 
            printf("No bytes are there to read");
        }       
        const char* message = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
        ssize_t bytes_sent = send(client_socket_fd, message, strlen(message), 0);
        if (bytes_sent == -1)
            std::perror("Send failed");
        else
            printf("------------------Hello message sent-------------------\n");
	    close(client_socket_fd);
        clients_count++;
    }

    // Close the socket when done
    close(socket_fd);
    return 0;
}