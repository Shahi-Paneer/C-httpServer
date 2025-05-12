#define _POSIX_C_SOURCE 200112L

/*
** showip.c -- show IP addresses for a host given on the command line
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h> // For close()

int main()
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int s, a;

    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    // === getaddrinfo() CHECK ===
    int test = getaddrinfo(NULL, "8080", &hints, &res);
    if (test != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(test));
        return 1;
    }
    else
    {
        printf("getaddrinfo: Success\n");
    }

    // === socket() CHECK ===
    s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (s == -1)
    {
        perror("Socket creation failed");
        freeaddrinfo(res); // Clean up
        return 1;
    }
    else
    {
        printf("Socket created successfully: %d\n", s);
    }

    // === bind() CHECK ===
    if (bind(s, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("Bind failed");
        freeaddrinfo(res); // Clean up
        close(s);          // Close the socket
        return 1;
    }
    else
    {
        printf("Bind successful!\n");
    }

    // === listen() CHECK ===
    if (listen(s, 10) == -1)
    {
        perror("Listen failed");
        freeaddrinfo(res); // Clean up
        close(s);          // Close the socket
        return 1;
    }
    else
    {
        printf("Listening on port 8080...\n");
    }

    // === accept() CHECK ===
    addr_size = sizeof their_addr;
    printf("Waiting for a connection...\n");

    a = accept(s, (struct sockaddr *)&their_addr, &addr_size);
    if (a == -1)
    {
        perror("Accept failed");
        freeaddrinfo(res); // Clean up
        close(s);          // Close the socket
        return 1;
    }
    else
    {
        printf("Client connected! Socket FD: %d\n", a);
    }

    // === Receive Data ===
    char buffer[1024];
    int bytes_received = recv(a, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received == -1)
    {
        perror("Receive failed");
    }
    else
    {
        buffer[bytes_received] = '\0'; // Null-terminate the string
        printf("Received from client: %s\n", buffer);
    }

    // === Send Response ===
    const char *response = "Message received!\n";
    if (send(a, response, strlen(response), 0) == -1)
    {
        perror("Send failed");
    }
    else
    {
        printf("Response sent to client.\n");
    }

    // Cleanup
    freeaddrinfo(res);
    close(a);
    close(s);

    return 0;
}
