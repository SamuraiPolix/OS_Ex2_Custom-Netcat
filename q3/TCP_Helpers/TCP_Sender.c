#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h> 
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// #define _DEBUG

#define SERVER_PORT 5050
#define SERVER_IP "127.0.0.1"

int main(int argc, char *argv[]){
    printf("Starting Sender...\n");

    // create a socket, ipv4, tcp type, tcp protocol (chosen automatically)
    int sock = -1;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sock < 0){
        perror("sock");
        exit(1);
    }

    struct sockaddr_in server;

    // reset address's memory before using it
    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;        // ipv4

    // Check the correct amount of args were received
    if (argc != 5){
        fprintf(stderr, "Usage: -ip <server_ip> -p <server_port>\n");
        fprintf(stderr, "Using default settings: -ip %s -p %d\n", SERVER_IP, SERVER_PORT);
        server.sin_port = htons(SERVER_PORT);
        if (inet_pton(AF_INET, SERVER_IP, &(server.sin_addr)) <= 0){
            perror("inet_pton");
            exit(1);
        }
    }
    else {
        int i = 1;
        // Take port, ip and algo from argv (if failed - stop program and ask for correct inputs)
        while (i < argc){
            if (argv[i][0] == '-') {
                if (!strcmp(argv[i], "-p")){
                    // Set server port
                    i++;
                    server.sin_port = htons(atoi(argv[i]));
                    #ifdef _DEBUG
                    printf("Server Port is set to: %d\n", atoi(argv[i]));
                    #endif
                }
                else if (!strcmp(argv[i], "-ip")){
                    // Set server ip
                    i++;
                    if (inet_pton(AF_INET, argv[i], &(server.sin_addr)) <= 0){
                        perror("inet_pton");
                        exit(1);
                    }
                    #ifdef _DEBUG
                    printf("Server IP is set to: %s\n", argv[i]);
                    #endif
                }
            }
            i++;
        }
    }

    // connect to server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == -1){
        perror("connect");
        close(sock);
        exit(1);
    }

    printf("Connected to the server.\n");

    printf("Enter the data to send (press Enter each time to send), or type 'exit' to close the connection:\n");

    while (1) {
        // Ask for input
        fflush(stdout);

        // Read input from user
        char input[100];
        fgets(input, sizeof(input), stdin);

        // Remove newline character from input
        input[strlen(input) - 1] = '\0';

        // Check if the input is "exit"
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // Send the input data
        int bytes_sent = send(sock, input, strlen(input) + 1, 0);
        if (bytes_sent == -1) {
            perror("send");
            close(sock);
            exit(1);
        } else if (bytes_sent == 0) {
            printf("Connection was closed prior to sending the data!\n");
            close(sock);
            exit(1);
        }
    }
    
    printf("Closing the TCP connection...\n");
    close(sock);

    printf("Sender end.\n");

    return 0;
}