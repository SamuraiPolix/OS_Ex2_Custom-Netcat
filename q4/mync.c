// THIS IS BUILT ON Q3 - we added UDP support

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>

#define SERVERPORT "4950"	// the port users will be connecting to
#define MYPORT "5050"
#define DESTADDR "127.0.0.1"
int numbytes=101;

// Declare functions:
int runProgram(char argv[]);

void *get_in_addr(struct sockaddr *sa);

// TCP
int createTcpTalker(char* address);
int createTcpListener(char* port);

// UDP
int createUdpTalker(char* address, struct sockaddr_in* server_addr);
int createUdpListener(char* port);

void handle_alarm(int sig) {
	printf("Timeout reached. Exiting all processes!\n");
	kill(0, SIGTERM); // Send SIGTERM signal to the process group
	exit(EXIT_SUCCESS);
}


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
	int isUdpTalker = 0;		// we handle udp talker differently than tcp
	int isUdpListener = 0;
	size_t timeout = 0;
	struct sockaddr_in server_addr;

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
		else if (strcmp(argv[i], "-i") == 0) {
			// take input from opened socket
			i++;
			if (strncmp(argv[i], "TCPS", 4) == 0) {
				// Open a server and listen on it
				inputSocket = createTcpListener(argv[i]+4);
			}
			else if (strncmp(argv[i], "TCPC", 4) == 0) {
				// open a client and listen on it
				inputSocket = createTcpTalker(argv[i]+4);
			}
			// Input needs to support UDP server only (not client)
			else if (strncmp(argv[i], "UDPS", 4) == 0) {
				// Open a server and listen on it
				inputSocket = createUdpListener(argv[i]+4);
				isUdpListener = 1;
			}
			else if (strncmp(argv[i], "UDPC", 4) == 0) {
				fprintf(stderr, "UDP Client doesn't support -i\n");
				exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(argv[i], "-o") == 0) {
			// send output to opened socket
			i++;
			if (strncmp(argv[i], "TCPS", 4) == 0) {
				// Open a server and talk on it
				outputSocket = createTcpListener(argv[i]+4);
			}
			else if (strncmp(argv[i], "TCPC", 4) == 0) {
				// Open a server and talk on it
				outputSocket = createTcpTalker(argv[i]+4);
			}
			// Output needs to support UDP client only (not server)
			else if (strncmp(argv[i], "UDPC", 4) == 0) {
				// open a client and listen on it
				outputSocket = createUdpTalker(argv[i]+4, &server_addr);
				isUdpTalker = 1;
			}
			else if (strncmp(argv[i], "UDPS", 4) == 0) {
				fprintf(stderr, "UDP Server doesn't support -o\n");
				exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(argv[i], "-b") == 0) {
			// send output and input to 2 opened sockets
			i++;
			if (strncmp(argv[i], "TCPS", 4) == 0) {
				// Open a server, listen and talk on it
				inputSocket = outputSocket = createTcpListener(argv[i]+4);
				isUdpListener = 1;
			}
			else if (strncmp(argv[i], "TCPC", 4) == 0) {
				// Open a server and talk on it
				inputSocket = outputSocket = createTcpTalker(argv[i]+4);
			}
			// UDP doesn't support -b
			else if (strncmp(argv[i], "UDPS", 4) == 0) {
				fprintf(stderr, "UDP doesn't support -b\n");
				exit(EXIT_FAILURE);
				// Open a server and listen on it
				// inputSocket = outputSocket = createUdpListener(argv[i]+4);
				// isUdpListener = 1;
				// isUdpTalker = 1;
			}
			else if (strncmp(argv[i], "UDPC", 4) == 0) {
				fprintf(stderr, "UDP doesn't support -b\n");
				exit(EXIT_FAILURE);
				// open a client and listen on it
				// inputSocket = outputSocket = createUdpTalker(argv[i]+4, &server_addr);
				// isUdpListener = 1;
				// isUdpTalker = 1;
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
		// if not udp, simply redirect stdin
		if (isUdpListener != 1){
			dup2(inputSocket, STDIN_FILENO);
		}
		else {	// if its UDP, we need to handle it differently - fork a new process to handle the redirection
			// create pipe to use between udp listener and prog exec
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
				struct sockaddr_in client_addr;
				socklen_t addr_len = sizeof(client_addr);
				while (1) {
					// set alarm for timeout
					if (timeout != 0){
						alarm(timeout);
					}
					int numbytes = recvfrom(inputSocket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
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
		// if not udp, simply redirect stdout
		if (isUdpTalker != 1){
			dup2(outputSocket, STDOUT_FILENO);
		}
		else {	// if its UDP, we need to handle it differently - fork a new process to handle the redirection
			// if -e needs to run - create pipe to use between udp talker and prog exec. else - no need for pipe
			if (command != NULL && pipe(outputPipe) == -1) {
				perror("pipe: output process.");
				exit(EXIT_FAILURE);
			}
			if ((outputPid = fork()) == 0) {
				close(outputPipe[1]);	// close the write end of the pipe
				if (command != NULL){
					dup2(outputPipe[0], STDIN_FILENO);
				}

				// using server_addr from createUdpTalker
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
					sendto(outputSocket, buffer, bytes_received, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
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

int createTcpTalker(char* address)
{ 
	// address is in the format of "IP,PORT", separate them
	char* temp = address;
	while (*temp != ',' && *temp != '\0') {
		temp++;
	}
	if (*temp == '\0') {
		fprintf(stderr, "Invalid address.\n");
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
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}
	if (connect (sockfd, p->ai_addr, p->ai_addrlen) == -1) {
		perror("talker: connect");
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

int createTcpListener(char* port)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

    listen(sockfd, 3);

	printf("listener: waiting to accept connection...\n");

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
    if (client_sock < 0) {
        perror("accept");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server is up\n");

    return client_sock;
}

// Returns the server_addr in the pointer given
int createUdpTalker(char* address, struct sockaddr_in* server_addr)
{
	// address is in the format of "IP,PORT", separate them
	char* temp = address;
	while (*temp != ',' && *temp != '\0') {
		temp++;
	}
	if (*temp == '\0') {
		fprintf(stderr, "Invalid address.\n");
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
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		// copy address info to server_addr for later use (sendto in main)
        memcpy(server_addr, p->ai_addr, sizeof(struct sockaddr_in));

		break;
	}
	return sockfd;
}

int createUdpListener(char* port)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	return sockfd;
}