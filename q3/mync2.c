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

#define SERVERPORT "4950"	// the port users will be connecting to
#define MYPORT "5050"
#define DESTADDR "127.0.0.1"
int numbytes=101;

int runProgram(char argv[]);
void handle_connection(int sockfd, int is_input);

int createTcpTalker(char* address)
{ 
	// address is in the format of "IP,PORT", separate them
	char* temp = address;
	while (*temp != ',' && *temp != '\0') {
		temp++;
	}
	if (*temp == '\0') {
		fprintf(stderr, "Invalid address.\n");
		exit(1);
	}
	*temp = '\0';
	temp++;
	char* serverPort = temp;

	int sockfd;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct addrinfo *p;
	int rv;
	int numbytes;
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
	connect (sockfd, p->ai_addr, p->ai_addrlen);
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
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[numbytes];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

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
	return sockfd;
}


int main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("usage: ./mync\n"
				"Optional:\n"
				"-e <program arguments>: run a program with arguments\n"
				"-i <parameter>: take input from opened parameter\n"
				"-o <parameter>: send output to opened parameter\n\n"
				"Parameters:\n"
				"TCPS<PORT>: open a TCP server on port PORT\n"
				"TCPC<IP,PORT> / TCPC<hostname,port>: open a TCP client and connect to IP on port PORT\n");
		exit(1);
	}

	char buffer[numbytes];

	char *command = NULL;
	// store sockets (if arguments are given for them [-i will store a sockid into inputSocket, -o will store a sockid into outputSocket])
	int inputSocket = -1;
	int outputSocket = -1;

	// skiping the first because its the name of this program
	for (int i = 1; i<argc; i++) {
		if (strcmp(argv[i], "-e") == 0) {
			// run a program with arguments
			i++;	// skip command
			char *p = argv[i];		// run with a pointer to get until the end of the arguments (next '-' flag or end of line)
			while (*(p+1) != '-' && *p != '\0') {
				p++;
			}
			*p = '\0';	// null-terminate the arguments
			command = argv[i];	// save a pointer to the command for later run (after we go over all the argvs here)
		}
		if (strcmp(argv[i], "-i") == 0) {
			// take input from opened socket
			i++;
			if (strncmp(argv[i], "TCPS", 4) == 0) {
				// Open a server and listen on it
				inputSocket = createTcpListener(argv[i]+4);
			}
			// if (strncmp(argv[i], "TCPC", 4) == 0) {
			// 	// open a client and listen on it
			// 	inputSocket = createTcpListener();
			// }
		}
		if (strcmp(argv[i], "-o") == 0) {
			// send output to opened socket
			i++;
			if (strncmp(argv[i], "TCPS", 4) == 0) {
				// Open a server and talk on it
				outputSocket = createTcpTalker(argv[i]+4);
			}
			if (strncmp(argv[i], "TCPC", 4) == 0) {
				// Open a server and talk on it
				outputSocket = createTcpTalker(argv[i]+4);
			}
		}
	}

	if (inputSocket == -1 || outputSocket == -1) {
        runProgram(command);
		exit(0);
    }

	pid_t pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(1);
	}

    if (pid == 0) {  // child
		// if -i or -o was given, we need to redirect the input/output to/from the sockets
		if (inputSocket != -1) {
			dup2(inputSocket, STDIN_FILENO);
			close(STDIN_FILENO);
		}
		if (outputSocket != -1) {
			dup2(outputSocket, STDOUT_FILENO);
			close(STDOUT_FILENO);
		}
		runProgram(command);
		exit(0);
    }
	else {  // parent
        if (inputSocket != -1) {
            handle_connection(inputSocket, 1);
        }
        if (outputSocket != -1) {
            handle_connection(outputSocket, 0);
        }
    }


	// int sockfd = createUdpTalker();
	// close(STDOUT_FILENO);
	// dup(sockfd); 
	// sockfd=createUdpListener();
	// close(STDIN_FILENO);
	// dup(sockfd);
	// while (1) {
	// 	numbytes=read(STDIN_FILENO,buffer, 100);
	// 	buffer[numbytes++]=0;
	// 	write(STDOUT_FILENO, buffer, numbytes); 
	// } 
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
}

void handle_connection(int sockfd, int is_input) {
    char buffer[BUFSIZ];
    ssize_t numbytes;

    for(;;) {
        if (is_input) {
            numbytes = read(sockfd, buffer, sizeof(buffer) - 1);
            if (numbytes <= 0) {
                if (numbytes == 0) {
                    printf("server: socket %d hung up\n", sockfd);
                } else {
                    perror("recv");
                }
                close(sockfd);
                break;
            }
            buffer[numbytes] = '\0';
            printf("%s", buffer);
        } else {
            numbytes = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            if (numbytes <= 0) {
                if (numbytes == 0) {
                    printf("client: socket %d hung up\n", sockfd);
                } else {
                    perror("recv");
                }
                close(sockfd);
                break;
            }
            buffer[numbytes] = '\0';
            if (write(sockfd, buffer, numbytes) == -1) {
                perror("send");
                close(sockfd);
                break;
            }
        }
    }
}