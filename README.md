# Exercise 2: Operating Systems for Computer Science - A Type of Netcat

### Overview

Developed a custom-made version of Netcat that uses sockets to:
- Show output/input of executed program, in another terminal (using sockets and pipes)
- Chat between terminals (using sockets and pipes)

Netcat is a versatile networking tool used for interacting with network sockets. For more information on using netcat, refer to the `man nc` command.

We also developed a dummy Tic-Tac-Toe AI and used it to test our work.

### MyNetcat - (90 Points)
**Folders: `q2, q6`**


Implement `mync` to redirect input/output of a specified program.
Supporting the following connections:
- TCP
- UDP
- Unix Domain Sockets (UDS)

#### Usage

The final code can be found under the `q6` folder.

If you intend to use the Tic-Tac-Toe program from `q1` please make sure to compile it (you can run `make` in the main directory to compile all questions).
```sh
./mync
      Optional:
      -e <program arguments>: run a program with arguments
      -i <parameter>: take input from opened parameter
      -o <parameter>: send output to opened parameter
      -b <parameter>: send output and input to opened parameter
      -t <timeout>: set timeout for UDP connection. defaults to 0

      Parameters:
      TCPS<PORT>: open a TCP server on port PORT
      TCPC<IP,PORT> / TCPC<hostname,port>: open a TCP client and connect to IP on port PORT
      UDPS<PORT>: open a UDP server on port PORT
      UDPC<IP,PORT> / UDPC<hostname,port>: open a UDP client and connect to IP on port PORT
      UDSSD<path>: open a Unix Domain Sockets Datagram server on path\n"
      UDSCD<path>: open a Unix Domain Sockets Datagram client and connect to path
      UDSSS<path>: open a Unix Domain Sockets Stream server on path
      UDSCS<path>: open a Unix Domain Sockets Stream client and connect to path
```

#### Examples

[A video of all possible examples and combinations can be found here (CLICK HERE)
](https://youtu.be/0bsxJbdX9is?si=YFWeWXE-niuzjcT8)

1. Starts a TCP server on port 4050 for input; output goes to stdout.

```sh
mync -e "ttt 123456789" -i TCPS4050
```

2. Starts a TCP server on port 4050 for both input and output.

```sh
mync -e "ttt 123456789" -b TCPS4050
```

3. TCP server on port 4050 for input, output sent to TCP client at localhost:4455.

```sh
mync -e "ttt 123456789" -i TCPS4050 -o TCPClocalhost,4455
```

4. UDP server on port 4050 with a 10-second timeout.

```sh
mync -e "ttt 123456789" -i UDPS4050 -t 10
```
5. UDP server on port 4050, echoing messages back to the sender, with a 12-second timeout.

```sh
mync -e "ttt 123456789" -b UDPS4050 -t 12
```



### Tic-Tac-Toe AI (10 Points)
**Folder: `q1`**

The program receives a 9-digit strategy number as a parameter (e.g., `198345762`), which must include each digit from 1 to 9 exactly once. 
If the input is invalid (missing/duplicate digits, includes 0, incorrect length), print `Error\n` and exit.

#### Dummy AI Strategy

- **MSD/LSD:** Most/Least Significant Digit
- The program prioritizes moves based on digit significance.
- Always choose the highest priority slot available.
- If only one slot remains, choose the LSD slot.

#### Gameplay

1. Program plays first by choosing the highest priority slot and prints the move.
2. The human player responds with a move (1-9).
3. The game continues until a win, loss, or draw.

