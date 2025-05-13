#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h> // For close()

char *getFile(char path[])
{

    if (access(path, F_OK) == 0)
    {
        // file exists
        printf("PATH EXISTS!\n");
        return("wassgood");
    }
    else
    {
        // file doesn't exist
        printf("Path not found!\n");
        printf("DFDFDFFDF");

        FILE *fptr;

        char notFoundPath[100] = "./public/404.html";

        // Open a file in read mode
        fptr = fopen(notFoundPath, "r");

        // Get the file size
        struct stat st;
        if (stat(notFoundPath, &st) == -1)
        {
            perror("Stat failed");
            fclose(fptr);
            return NULL;
        }

        // Allocate memory for the file content (+1 for null terminator)
        char *content = malloc(st.st_size + 1);
        if (!content)
        {
            perror("Memory allocation failed");
            fclose(fptr);
            return NULL;
        }

        // Read the file into the buffer
        size_t bytes_read = fread(content, 1, st.st_size, fptr);
        content[bytes_read] = '\0'; // Null-terminate the string

        fclose(fptr);

        // Print the file content
        printf("%s", content);

        return content;
    }
}

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

    // Set SO_REUSEADDR to allow immediate rebinding:
    int opt = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        return 1;
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

    while (1)
    {

        a = accept(s, (struct sockaddr *)&their_addr, &addr_size);

        char *string = NULL;
        string = (char *)calloc(1, sizeof(char));

        char httpHeader[42] = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";

        int responseLength;

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

            char *method = strtok(buffer, " ");
            char *path = strtok(NULL, " ");

            printf("Method: %s\n", method);
            printf("Path: %s\n", path);

            responseLength = strlen(getFile(path)) + 42; //add length of http header
            string = (char*)calloc(responseLength + 1, sizeof(char));
            string = strcat(httpHeader, getFile(path));
        }

        // === Send Response ===

        const char *response = string;
        if (send(a, response, strlen(response), 0) == -1)
        {
            perror("Send failed");
        }
        else
        {
            printf("Response sent to client.\n");
        }

        close(a);
    }

    // Cleanup
    freeaddrinfo(res);
    close(s);

    return 0;
}
