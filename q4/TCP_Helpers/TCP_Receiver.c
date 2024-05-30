#include <arpa/inet.h>
#include <netinet/tcp.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define _DEBUG

#define SERVER_PORT 6000        // The port the server is listening on
#define CLIENTS 1           // Max allowed clients in queue


int main(int argc, char *argv[]) {
    printf("Starting Receiver...\n");

    int sock = -1;
    // create a socket, ipv4, tcp type, tcp protocol (chosen automatically)
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0){
        perror("sock");
        exit(1);
    }

    struct sockaddr_in server, client;
    // reset address's memory before using it
    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;        // ipv4
    server.sin_addr.s_addr = INADDR_ANY;        // accept connections from any ip

    // Check the correct amount of args were received
    if (argc != 3){
        fprintf(stderr, "Usage: -p <server_port>\n");
        fprintf(stderr, "Using default settings: -p %d\n", SERVER_PORT);
        server.sin_port = htons(SERVER_PORT);
    }
    else {
        int i = 1;
        // Take port, algo from argv (if failed - stop program and ask for correct inputs)
        while (i < argc){
            if (argv[i][0] == '-') {
                if (!strcmp(argv[i], "-p")){
                    // Set port
                    i++;
                    server.sin_port = htons(atoi(argv[i]));
                    #ifdef _DEBUG
                    printf("Port is set to: %d\n", atoi(argv[i]));
                    #endif
                }
            }
            i++;
        }
    }
    
    // bind address and port to the server's socket
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) == -1){
        close(sock);
        if (errno == EADDRINUSE) {
            // Deal with "Address already in use" error
            fprintf(stderr, "bind: Address already in use\n");
            printf("Fixing error...\n");
            int yes = 1;
            if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1){
                fprintf(stderr, "Couldn't fix the error, Exiting...\n");
                perror("setsockopt");
                close(sock);
                exit(1);
            }
            printf("Error is fixed.\nPlease restart the program!\n");
        }
        else{
            perror("bind");
        }
        exit(1);
    }

    // listen for connections. allowing CLIENTS clients in queue
    if (listen(sock, CLIENTS) == -1){
        perror("listen");
        close(sock);
        exit(1);
    }

    printf("Waiting for TCP connection...\n");

    int client_len = sizeof(client);

    // accept connection and store client's socket id
    int sock_client = accept(sock, (struct sockaddr *)&client, (socklen_t*)&client_len);
    if (sock_client == -1) {
        perror("accept");
        close(sock);
        exit(1);
    }

    printf("Sender connected, beginning to receive data...\n");

    int bytes_received;
    char buffer[BUFSIZ] = {0};

    while(1) {
        bytes_received = recv(sock_client, buffer, BUFSIZ, 0);
        if (bytes_received <= -1){
            perror("recv");
            close(sock);
            exit(1);
        }
        else if (bytes_received == 0){
            printf("Connection was closed prior to receiving the data!\n");
            close(sock);
            exit(1);
        }

        // Reset buffer before more data is received
        memset(buffer, 0, BUFSIZ);
    };

    // Close connection
    close(sock);
    
    printf("Receiver end.\n");
    
    return 0;
}