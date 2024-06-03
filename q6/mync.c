// THIS IS BUILT ON Q3 - we added UDP support

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
#include <netdb.h>			// getaddrinfo()
#include <sys/wait.h>		// waitpid()
#include <signal.h>			// alarm()
#include <sys/un.h>			// Unix Domain Sockets

#define SERVERPORT "4950"	// the port users will be connecting to
#define MYPORT "5050"
#define DESTADDR "127.0.0.1"
#define MAX_CLIENTS 10
int numbytes=101;

// Declare functions:
int runProgram(char argv[]);

void *get_in_addr(struct sockaddr *sa);

// TCP
int createTCPClient(char* address);
int createTCPServer(char* port);

// UDP
int createUDPClient(char* address, struct sockaddr_in* server_addr_udp);
int createUDPServer(char* port);
void handle_alarm(int sig) {
	printf("Timeout reached. Exiting all processes!\n");
	kill(0, SIGTERM); // Send SIGTERM signal to the process group
	exit(EXIT_SUCCESS);
}

// UDS - Unix Domain Sockets (Stream and Datagram)
// DATAGRAM
int createUDSSD(const char *path);
int createUDSCD(const char *path, struct sockaddr_un* server_addr_uds);
// STREAM
int createUDSSS(const char *path);
int createUDSCS(const char *path);


int main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("usage: ./mync\n"
				"Optional:\n"
				"-e <program arguments>: run a program with arguments\n"
				"-i <parameter>: take input from opened parameter\n"
				"-o <parameter>: send output to opened parameter\n"
				"-b <parameter>: send output and input to opened parameter\n"
				"-t <timeout>: set timeout for UDP connection. defaults to 0\n\n"
				"Parameters:\n"
				"TCPS<PORT>: open a TCP server on port PORT\n"
				"TCPC<IP,PORT> / TCPC<hostname,port>: open a TCP client and connect to IP on port PORT\n");
		exit(EXIT_FAILURE);
	}
	char *command = NULL;
	// store sockets (if arguments are given for them [-i will store a sockid into inputSocket, -o will store a sockid into outputSocket])
	int inputSocket = -1;
	int outputSocket = -1;
	// UDP
	int isDatagramClient = 0;		// we handle datagram client differently than tcp
	int isDatagramServer = 0;
	size_t timeout = 0;
	// UDS + UDP
	struct sockaddr server_addr;	// generic (not _in or _um) to allow usage for both uds and udp (udp uses sockaddr_in, uds uses sockaddr_un)

	// skiping the first because its the name of this program
	for (int i = 1; i<argc; i++) {
		if (strcmp(argv[i], "-e") == 0) {
			// run a program with arguments
			i++;	// skip -e
			char *p = argv[i];		// run with a pointer to get until the end of the arguments (next '-' flag or end of line)
			while (*(p+1) != '-' && *p != '\0') {
				p++;
			}
			*p = '\0';	// null-terminate the arguments
			command = argv[i];	// save a pointer to the command for later run (after we go over all the argvs here)
		}
		if (strcmp(argv[i], "-t") == 0) {
			// set timeout for udp
			i++;	// skip -t
			timeout = atoi(argv[i]);
			signal(SIGALRM, handle_alarm);	// set alarm handler
		}
		// -------------- '-i' --------------
		else if (strcmp(argv[i], "-i") == 0) {
			// take input from opened socket
			i++;
			// -------------- TCP --------------
			if (strncmp(argv[i], "TCPS", 4) == 0) {
				// Open a server and listen on it
				inputSocket = createTCPServer(argv[i]+4);
			}
			else if (strncmp(argv[i], "TCPC", 4) == 0) {
				// open a client and listen on it
				inputSocket = createTCPClient(argv[i]+4);
			}
			// -------------- UDP --------------
			// Input needs to support UDP server only (not client)
			else if (strncmp(argv[i], "UDPS", 4) == 0) {
				// Open a server and listen on it
				inputSocket = createUDPServer(argv[i]+4);
				isDatagramServer = 1;
			}
			else if (strncmp(argv[i], "UDPC", 4) == 0) {
				fprintf(stderr, "UDP Client doesn't support -i\n");
				exit(EXIT_FAILURE);
			}
			// -------------- UDS --------------
			else if (strncmp(argv[i], "UDSSD", 5) == 0) {
				inputSocket = createUDSSD(argv[i]+5);
			}
			else if (strncmp(argv[i], "UDSCD", 5) == 0) {
				fprintf(stderr, "UDS Datagram Client doesn't support -i\n");
				exit(EXIT_FAILURE);
			}
			else if (strncmp(argv[i], "UDSSS", 5) == 0) {
				inputSocket = createUDSSS(argv[i]+5);
				// isDatagramServer = 1;
			}
			else if (strncmp(argv[i], "UDSCS", 5) == 0) {
				// Open a server and listen on it
				inputSocket = createUDSCS(argv[i]+5);
				// isDatagramServer = 1;
			}
		}
		// -------------- '-o' --------------
		else if (strcmp(argv[i], "-o") == 0) {
			// send output to opened socket
			i++;
			// -------------- TCP --------------
			if (strncmp(argv[i], "TCPS", 4) == 0) {
				// Open a server and talk on it
				outputSocket = createTCPServer(argv[i]+4);
			}
			else if (strncmp(argv[i], "TCPC", 4) == 0) {
				// Open a server and talk on it
				outputSocket = createTCPClient(argv[i]+4);
			}
			// -------------- UDP --------------
			// Output needs to support UDP client only (not server)
			else if (strncmp(argv[i], "UDPC", 4) == 0) {
				// open a client and listen on it
				outputSocket = createUDPClient(argv[i]+4, (struct sockaddr_in *)&server_addr);
				isDatagramClient = 1;
			}
			else if (strncmp(argv[i], "UDPS", 4) == 0) {
				fprintf(stderr, "UDP Server doesn't support -o\n");
				exit(EXIT_FAILURE);
			}
			// -------------- UDS --------------
			else if (strncmp(argv[i], "UDSSD", 5) == 0) {
				fprintf(stderr, "UDS Datagram Server doesn't support -o\n");
				exit(EXIT_FAILURE);
			}
			else if (strncmp(argv[i], "UDSCD", 5) == 0) {
				// open a client and listen on it
				outputSocket = createUDSCD(argv[i]+5, (struct sockaddr_un *)&server_addr);
				isDatagramClient = 1;
			}
			else if (strncmp(argv[i], "UDSSS", 5) == 0) {
				// Open a server and listen on it
				outputSocket = createUDSSS(argv[i]+5);
			}
			else if (strncmp(argv[i], "UDSCS", 5) == 0) {
				// Open a server and listen on it
				outputSocket = createUDSCS(argv[i]+5);
				// isDatagramServer = 1;
			}
		}
		// -------------- '-b' --------------
		else if (strcmp(argv[i], "-b") == 0) {
			// send output and input to 2 opened sockets
			i++;
			// -------------- TCP --------------
			if (strncmp(argv[i], "TCPS", 4) == 0) {
				// Open a server, listen and talk on it
				inputSocket = outputSocket = createTCPServer(argv[i]+4);
			}
			else if (strncmp(argv[i], "TCPC", 4) == 0) {
				// Open a server and talk on it
				inputSocket = outputSocket = createTCPClient(argv[i]+4);
			}
			// -------------- UDP --------------
			// UDP doesn't support -b
			else if (strncmp(argv[i], "UDPS", 4) == 0) {
				fprintf(stderr, "UDP doesn't support -b\n");
				exit(EXIT_FAILURE);
			}
			else if (strncmp(argv[i], "UDPC", 4) == 0) {
				fprintf(stderr, "UDP doesn't support -b\n");
				exit(EXIT_FAILURE);
			}
			// -------------- UDS --------------
			else if (strncmp(argv[i], "UDSSD", 5) == 0) {
				fprintf(stderr, "UDS Datagram Server doesn't support -b\n");
				exit(EXIT_FAILURE);
			}
			else if (strncmp(argv[i], "UDSCD", 5) == 0) {
				fprintf(stderr, "UDS Datagram Client doesn't support -b\n");
				exit(EXIT_FAILURE);
			}
			else if (strncmp(argv[i], "UDSSS", 5) == 0) {
				// Open a server and listen on it
				outputSocket = inputSocket = createUDSSS(argv[i]+5);
				// isDatagramServer = 1;
			}
			else if (strncmp(argv[i], "UDSCS", 5) == 0) {
				// Open a server and listen on it
				outputSocket = inputSocket = createUDSCS(argv[i]+5);
				// isDatagramServer = 1;
			}
		}
	}

    pid_t inputPid = -1, outputPid = -1, execPid = -1;
    int inputPipe[2] = {-1, -1};
	int outputPipe[2] = {-1, -1};

	/* When -b (or -i and -o together) are given
	 * We need to run 3 things at the same time - input, output and the program itself
	 * So we use 3 forks to run them in parallel and pipes between them to redirect input/output
	 */

	// Fork and handle -i (input redirection)	
	if (inputSocket != -1) {
		// if not Datagram, simply redirect stdin
		if (isDatagramServer != 1){
			dup2(inputSocket, STDIN_FILENO);
		}
		else {	// if its Datagram, we need to handle it differently - fork a new process to handle the redirection
			// create pipe to use between datagram server and prog exec
			if (command != NULL && pipe(inputPipe) == -1) {
				perror("pipe: input process.");
				exit(EXIT_FAILURE);
			}
			if ((inputPid = fork()) == 0) {
				close(inputPipe[0]);	// close the read end of the pipe
				if (command != NULL){
					dup2(inputPipe[1], STDOUT_FILENO);
				}
				char buffer[1024];
				// struct sockaddr client_addr;
				// socklen_t addr_len = sizeof(client_addr);
				while (1) {
					// set alarm for timeout
					if (timeout != 0){
						alarm(timeout);
					}
					// TODO can change client address to NULL? no need to store it
					// int numbytes = recvfrom(inputSocket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
					int numbytes = recvfrom(inputSocket, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
					alarm(0);	// cancel alarm - we received data
					if (numbytes == -1) {
						perror("recvfrom: input process.");
						exit(EXIT_FAILURE);
					}
					buffer[numbytes] = '\0';
					// printf("Received: %s\n", buffer);
					// Write the received data to stdout or it's replacement
					// write(STDIN_FILENO, buffer, numbytes);
					write(STDOUT_FILENO, buffer, numbytes);
					fflush(stdout);  // Ensure the buffer is flushed immediately
				}
				close(inputPipe[1]);
			}
			else if (inputPid < 0) {		// forking failed
				perror("fork: input process.");
				// close sockets before exiting
				if (inputSocket != -1) {
					close(inputSocket);
				}
				if(outputSocket != -1) {
					close(outputSocket);
				}
				exit(EXIT_FAILURE);
			}
		}
	}

	// Fork and handle -o (output redirection)	
	if (outputSocket != -1) {	
		// if not Datagram, simply redirect stdout
		if (isDatagramClient != 1){
			dup2(outputSocket, STDOUT_FILENO);
		}
		else {	// if its Datagram, we need to handle it differently - fork a new process to handle the redirection
			// if -e needs to run - create pipe to use between datagram client and prog exec. else - no need for pipe
			if (command != NULL && pipe(outputPipe) == -1) {
				perror("pipe: output process.");
				exit(EXIT_FAILURE);
			}
			if ((outputPid = fork()) == 0) {
				close(outputPipe[1]);	// close the write end of the pipe
				if (command != NULL){
					dup2(outputPipe[0], STDIN_FILENO);
				}

				// using server_addr (from udp or uds, that we got earlier) to send data to the server
				char buffer[1024];
				int bytes_received = 0;
				// keep reading from stdout until timeout from alarm
				while (1)
				{
					// set alarm for timeout
					if (timeout != 0){
						alarm(timeout);
					}
					bytes_received = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
					alarm(0);	// cancel alarm - we received data
					if (bytes_received < 0) {
						perror("read: output process.");
						exit(EXIT_FAILURE);
					}
					buffer[bytes_received] = '\0';
					sendto(outputSocket, buffer, bytes_received, 0, &server_addr, sizeof(server_addr));
					// fflush(stdout);
				}
				close(outputSocket);
				close(outputPipe[0]);
			}
			else if (outputPid < 0) {		// forking failed
				perror("fork: output process.");
				// close sockets before exiting
				if (inputSocket != -1) {
					close(inputSocket);
				}
				if(outputSocket != -1) {
					close(outputSocket);
				}
				exit(EXIT_FAILURE);
			}
		}
	}

	// Fork and handle -e (program exec)	
	if (command != NULL) {
		if ((execPid = fork()) == 0) {
			if (inputPipe[0] != -1) {
				close(inputPipe[1]); 			// Close unused write end
				dup2(inputPipe[0], STDIN_FILENO); 		// Redirect stdin to read end of the pipe
			}
			if (outputPipe[1] != -1) {
				close(outputPipe[0]); 			// Close unused read end
				dup2(outputPipe[1], STDOUT_FILENO); 	// Redirect stdout to write end of the pipe
			}
			runProgram(command);
			// close sockets before exiting
			if (inputSocket != -1) {
				close(inputSocket);
			}
			if(outputSocket != -1) {
				close(outputSocket);
			}
			exit(EXIT_SUCCESS);
		}
		else if (execPid < 0) {		// forking failed
			perror("fork: program exec process.");
			// close sockets before exiting
			if (inputSocket != -1) {
				close(inputSocket);
			}
			if(outputSocket != -1) {
				close(outputSocket);
			}
			exit(EXIT_FAILURE);
		}
	}

	// Parent process: wait for program exec to finish
	if (execPid > 0) {
		int status;
		waitpid(execPid, &status, 0);
		if (status != 0) {
			fprintf(stderr, "Exec process failed\n");
		}
	}
	
    if (inputSocket != -1) {
		close(inputSocket);
	}
	if(outputSocket != -1) {
		close(outputSocket);
	}	
	
	return 0;
}

#define MAX_ARGS_SIZE 100
int runProgram(char argv[]) {
	// Code is taken from q2:
	char *p = argv;  // a pointer used to extract the command and arguments from argv
    char *args[MAX_ARGS_SIZE];      // store pointers to the command and arguments
    int i = 0;      // index for args
    
    // skip leading spaces
    while (*p == ' ') {
        p++;
    }
    
    // we are at the start of the command to execute - place args[0] pointer here, increment i
    args[i++] = p;
    
    // skip spaces and make sure we are not going off
    while (*p != ' ' && *p != '\0') {
        p++;
    }

    *p = '\0';  // null-terminate the program name

    p++;    // move to the next character

    // extract the arguments
    while (*p != '\0') {
        // skip leading spaces
        while (*p == ' ') {
            p++;
        }

        // if we reach the end, break
        if (*p == '\0') {
            break;
        }

        // p is now placed at the start of the argument - place args[i] pointer here, increment i
        args[i++] = p;

        // go until the end of the argument
        while (*p != ' ' && *p != '\0') {
            p++;
        }

        if (*p != '\0') {   // if we are not at the end
            *p = '\0';  // null-terminate the argument
            p++;    // move to the next character
        }
        // else, we are at the end of the command - the loop will break
    }

    args[i] = NULL; // null-terminate the arguments list, for execvp to work

    // execute command
    execvp(args[0], args);

    // if execvp returns (this thread didnt die), there was an error
    perror("execvp");

	return EXIT_SUCCESS;
}

int createTCPClient(char* address)
{ 
	// address is in the format of "IP,PORT", separate them
	char* temp = address;
	while (*temp != ',' && *temp != '\0') {
		temp++;
	}
	if (*temp == '\0') {
		fprintf(stderr, "TCP Client: Invalid address.\n");
		exit(EXIT_FAILURE);
	}
	*temp = '\0';
	temp++;
	char* serverPort = temp;

	int sockfd;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct addrinfo *p;
	int rv;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_STREAM;		// TCP

	if ((rv = getaddrinfo(address, serverPort, &hints, &servinfo)) != 0) {
		fprintf(stderr, "TCP Client: getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("TCP Client: socket");
			continue;
		}
		break;
	}
	if (connect (sockfd, p->ai_addr, p->ai_addrlen) == -1) {
		perror("TCP Client: connect");
		exit(EXIT_FAILURE);
	}
	
	return sockfd;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int createTCPServer(char* port)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "TCP Server: getaddrinfo: %s\n", gai_strerror(rv));
		exit(EXIT_FAILURE);
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("TCP Server: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("TCP Server: bind");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "TCP Server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo);

    if (listen(sockfd, MAX_CLIENTS) == -1) {
		perror("TCP Server: listen");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	printf("TCP Server: waiting to accept connection...\n");

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
    if (client_sock < 0) {
        perror("TCP Server: accept");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("TCP Server is up\n");

    return client_sock;
}

// Returns the server_addr_udp in the pointer given
int createUDPClient(char* address, struct sockaddr_in* server_addr_udp)
{
	memset(server_addr_udp, 0, sizeof *server_addr_udp);
	// address is in the format of "IP,PORT", separate them
	char* temp = address;
	while (*temp != ',' && *temp != '\0') {
		temp++;
	}
	if (*temp == '\0') {
		fprintf(stderr, "UDP Client: Invalid address.\n");
		exit(EXIT_FAILURE);
	}
	*temp = '\0';
	temp++;
	char* serverPort = temp;

	int sockfd;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct addrinfo *p;
	int rv;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;		// UDP

	if ((rv = getaddrinfo(address, serverPort, &hints, &servinfo)) != 0) {
		fprintf(stderr, "UDP Client: getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("UDP Client: socket");
			continue;
		}

		// copy address info to server_addr_udp for later use (sendto in main)
        memcpy(server_addr_udp, p->ai_addr, sizeof(struct sockaddr_in));

		break;
	}
	return sockfd;
}

int createUDPServer(char* port)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "UDP Server: getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("UDP Server: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("UDP Server: bind");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "UDP Server: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	return sockfd;
}
// Unix Domain Sockets Datagram Server
int createUDSSD(const char *path){
	unlink(path);
	int serverSocket;
    struct sockaddr_un serverAddr;

    // create a Unix domain socket
    if ((serverSocket = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        perror("UDSSD: socket");
        exit(EXIT_FAILURE);
    }

	memset(&serverAddr, 0, sizeof(serverAddr));

    // set up server family and name (path)
    serverAddr.sun_family = AF_UNIX;
    strncpy(serverAddr.sun_path, path, sizeof(serverAddr.sun_path) - 1);

    // bind socket to specified path
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("UDSSD: bind");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("UDSSD is up\n");

	return serverSocket;
}
// Unix Domain Sockets Datagram Client
int createUDSCD(const char *path, struct sockaddr_un* serverAddr){
	int clientSocket;

    // create a Unix domain socket
    if ((clientSocket = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        perror("UDSCD: socket");
        exit(EXIT_FAILURE);
    }

	// make sure address is empty
	memset(serverAddr, 0, sizeof(*serverAddr));

    // set up server family and name (path)
    serverAddr->sun_family = AF_UNIX;
    strncpy(serverAddr->sun_path, path, sizeof(serverAddr->sun_path) - 1);

	return clientSocket;		// address returned in pointer serverAddr
}
// Unix Domain Sockets Stream Server
int createUDSSS(const char *path){
	unlink(path);
	int serverSocket;
    struct sockaddr_un serverAddr;

    // create a Unix domain socket
    if ((serverSocket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("UDSSD: socket");
        exit(EXIT_FAILURE);
    }

	memset(&serverAddr, 0, sizeof(serverAddr));

    // set up server family and name (path)
    serverAddr.sun_family = AF_UNIX;
    strncpy(serverAddr.sun_path, path, sizeof(serverAddr.sun_path) - 1);

    // bind socket to specified path
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("UDSSS: bind");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
	
	// listen for incoming connections
	if (listen(serverSocket, MAX_CLIENTS) == -1) {
		perror("UDSSS: listen");
		close(serverSocket);
		exit(EXIT_FAILURE);
	}

	printf("UDSSS: waiting to accept connection...\n");

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock = accept(serverSocket, (struct sockaddr *) &client_addr, &client_len);
    if (client_sock < 0) {
        perror("UDSSS: accept");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("UDSSS is up\n");
	// TODO understand how to close server socket outside of this function - close and unlink
	return client_sock;
}
// Unix Domain Sockets Stream Client
int createUDSCS(const char *path){
	int clientSocket;
    struct sockaddr_un serverAddr;

    // create a Unix domain socket
    if ((clientSocket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("UDSCS: socket");
        exit(EXIT_FAILURE);
    }

	memset(&serverAddr, 0, sizeof(serverAddr));

    // set up server family and name (path)
    serverAddr.sun_family = AF_UNIX;
    strncpy(serverAddr.sun_path, path, sizeof(serverAddr.sun_path) - 1);

	// connect to server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
		perror("UDSCS: connect");
		close(clientSocket);
		exit(EXIT_FAILURE);
	}

	printf("UDSCS connected\n");

	return clientSocket;
}