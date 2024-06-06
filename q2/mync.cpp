#include <cstdio>
#include <unistd.h>
#include <cstring>

#define MAX_ARGS_SIZE 256

int main(int argc, char* argv[]) {
    // If not enough argc or argv[1] not -e
    if (argc < 3 || strcmp(argv[1], "-e") != 0) {
        printf("Usage: %s -e \"<program> <arguments>\" \n", argv[0]);
        return 1;
    }

    char *p = argv[2];  // a pointer used to extract the command and arguments from argv
    printf("argv[2]: %s\n", p); // print the command and arguments (for debugging purposes
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
    return 1;
}
