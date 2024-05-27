# Exercise 2: Operating Systems for Computer Science - A Type of Netcat

**Version 1.2 - May 20, 2024**

## Overview

This README outlines the steps to complete Exercise 2, which involves developing a custom version of netcat and a simple AI for tic-tac-toe.

### Netcat Overview

Netcat is a versatile networking tool used for interacting with network sockets. For more information on using netcat, refer to the `man nc` command.

**Note:** There are several versions of netcat, and not all support the `-e` option used in this assignment. If your version lacks this option, consult online resources for alternatives.

## Step 1: Tic-Tac-Toe AI (10 Points)

### Objective

Develop the worst AI for tic-tac-toe (ttt) to restore the dignity of the human race by creating an extremely simple opponent.

### Board Representation

The tic-tac-toe board is represented as follows:

```
3 2 1
6 5 4
9 8 7
```

### Input

The program receives a 9-digit strategy number as a parameter (e.g., `198345762`), which must include each digit from 1 to 9 exactly once. 

**Validation:** If the input is invalid (missing/duplicate digits, includes 0, incorrect length), print `Error\n` and exit.

### Strategy

- **MSD/LSD:** Most/Least Significant Digit
- The program prioritizes moves based on digit significance.
- Always choose the highest priority slot available.
- If only one slot remains, choose the LSD slot.

### Gameplay

1. Program plays first by choosing the highest priority slot and prints the move.
2. The human player responds with a move (1-9).
3. The game continues until a win, loss, or draw:
   - **Win:** Print `I win\n` and terminate.
   - **Loss:** Print `I lost\n` and terminate.
   - **Draw:** Print `DRAW\n` and terminate.

## Step 2: MyNetcat - Step 1 (10 Points)

### Objective

Implement `mync` to redirect input/output of a specified program.

### Usage

```sh
mync -e "ttt 123456789"
```

This command runs the `ttt` program with the given argument.

## Step 3: MyNetcat - Step 2 (20 Points)

### Objective

Extend `mync` to support TCP server/client connections for input/output redirection.

### Parameters

- **-i:** Direct input
- **-o:** Direct output
- **-b:** Direct both input and output

### Examples

```sh
mync -e "ttt 123456789" -i TCPS4050
```

Starts a TCP server on port 4050 for input; output goes to stdout.

```sh
mync -e "ttt 123456789" -b TCPS4050
```

Starts a TCP server on port 4050 for both input and output.

```sh
mync -e "ttt 123456789" -i TCPS4050 -o TCPClocalhost,4455
```

TCP server on port 4050 for input, output sent to TCP client at localhost:4455.

## Step 3.5: MyNetcat - Step 2.5 (20 Points)

### Objective

Allow `mync` to read from standard input and write to standard output if `-e` is not specified.

### Example

Running `mync` from two terminals can facilitate a chat between them or test I/O against another instance using `-e`.

## Step 4: MyNetcat - Step 3 (10 Points)

### Objective

Add support for UDP server/client connections.

### Parameters

- **UDPS<PORT>:** UDP server on port
- **UDPC<IP, PORT>:** UDP client to IP/PORT

### Timeout

Implement a timeout using the `alarm(2)` function. If no timeout is specified, the server stops only when the executable ends.

### Examples

```sh
mync -e "ttt 123456789" -i UDPS4050 -t 10
```

UDP server on port 4050 with a 10-second timeout.

```sh
mync -e "ttt 123456789" -b UDPS4050 -t 12
```

UDP server on port 4050, echoing messages back to the sender, with a 12-second timeout.

## Step 5: MyNetcat - Step 4 (30 Points)

### Objective

Extend `mync` to support TCP IO MUX.

### Parameters

- **TCPMUXS<PORT>:** Start a TCP server supporting IO MUX on the specified port.

### Example

```sh
mync -e "ttt 123456789" -b TCPMUXS4050
```

Starts a TCP server with IO MUX on port 4050.

## Step 6: Unix Domain Sockets (30 Points)

### Objective

Add support for Unix domain sockets.

### Parameters

- **UDSSD<path>:** Unix domain socket server (datagram)
- **UDSCD<path>:** Unix domain socket client (datagram)
- **UDSSS<path>:** Unix domain socket server (stream)
- **UDSCS<path>:** Unix domain socket client (stream)

### Note

No need to support IO MUX for Unix domain sockets.

## Additional Instructions

1. Use appropriate methods to handle parameters and implement timeouts.
2. Include a code coverage report.
3. Provide a Makefile to build all steps.
4. The exercise contributes 10% to the final grade and 5% to the defense grade.
5. In version 1.1, step 2.5 was added, increasing the total score to 130 points. You may choose to skip either step 5 or step 6 and still achieve full marks. Completing both steps will not exceed 100 points.