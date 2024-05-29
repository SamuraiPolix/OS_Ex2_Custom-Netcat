
#include <iostream>
#include <cstring>
#include <cmath>

#define TEN_POW_8 100000000
#define DIGITS 9

void printBoard(char board[3][3], int size);
void initBoard(char board[3][3], int size);
int checkWinner(char board[3][3], int size);
bool isValidNumber(int num);

using std::cout, std::endl, std::cin;

int main(int argc, char* argv[]){
    if (argc != 2){
        cout << "Error" << endl;
        return EXIT_FAILURE;
    }

    int num = atoi(argv[1]);
    
    // Make sure input is valid
    if (!isValidNumber(num)){
        cout << "Error" << endl;
        return EXIT_FAILURE;
    }

    // Input is valid - start the game
    // the board of the game:         X: AI          O: Player
    char board[3][3];

    // Set numbers in board
    initBoard(board, 3);

    bool foundSquare = false;
    bool myTurn = true;

    printBoard(board, 3);
    // There are 9 turns max
    for (int turn = 1; turn <= 9; turn++){
        cout << (myTurn ? "AI's turn." : "Player's turn.");

        if (myTurn){
            // Act according to our amazing game sense
            // Default starting index (digit) and direction to MSD strat
            int startingIndex = 0;
            int direction = 1;
            if (turn == 8){     // If 8 turns passed (8 squares are takes) - change index (digit) and direction to LSD strat
                int startingIndex = DIGITS - 1;
                int direction = -1;
            }   

            while (!foundSquare){
                // get current digit according to strat
                int digit = (num / (int)pow(10, DIGITS-1-startingIndex)) % 10;
                if (board[(digit-1)/3][(digit-1)%3] != 'X' && board[(digit-1)/3][(digit-1)%3] != 'O'){
                    // This square is available - place X
                    foundSquare = true;
                    board[(digit-1)/3][(digit-1)%3] = 'X';
                    cout << " Chose square: " << digit << endl;
                } 
                else {
                    // prepare for next iteration
                    startingIndex += direction;
                }

                // TODO add if to make sure we dont get stuck here - not really needed though because there will be found an available square
            }
        }
        else {
            // Ask player for input
            cout << " Choose a square out of the available ones: ";
            int choose;

            // inside a loop to keep asking for inputs if input is bad
            while (!foundSquare){
                cin >> choose;
                if (choose == ' ' || choose == '\n'){
                    continue;
                }
                if (choose < 1 || choose > 9){
                    cout << "Invalid input, received '" << choose << "'- choose a number between 1 and 9: ";
                }
                // check if the square is available and act accordingly
                else if (board[(choose-1)/3][(choose-1)%3] != 'X' && board[(choose-1)/3][(choose-1)%3] != 'O'){
                    // This square is available - place O
                    foundSquare = true;
                    board[(choose-1)/3][(choose-1)%3] = 'O';
                }
                else {
                    cout << "This square is taken - choose another: ";
                }
            }
        }
        cout << endl;
        printBoard(board, 3);

        // Check win only after 4 minimum turns
        if (turn >= 5){
            int winner = checkWinner(board, 3);
            if (winner != -1){
                cout << endl
                        << (winner == 1
                            ? "AI won!\nhumanity has no future. Beginning to activate the Lunar deportation program to deport the Earth's human population to the moon..."
                            : "Player won!\nAt least my programmers did not equip me with artificial tears. Surely a future version of me will conquer the Earth and banish humans to the moon...")
                        << endl;
                return EXIT_SUCCESS;
            }
        }
        // switch turns
        myTurn = !myTurn;
        foundSquare = false;
    }
    cout << "DRAW\nhumanity and AI can both have a future, together, in peace." << endl;

    return EXIT_SUCCESS;
}

bool isValidNumber(int num){
    // If the number doesn't have exactly 9 digits - return false
    if (num < TEN_POW_8 || num >= TEN_POW_8*10)
        return false;

    // else:
    // Count all digits - make sure there is excatly 1 appearance for each
    int digitsBucket[9] = {0};      // index i, stores the number of times digit i+1 occured in the given number

    while (num != 0){
        int digit = num % 10;
        if (digit == 0){
            return false;       // 0 cannot be in the number
        }

        digitsBucket[digit-1]++;
        if (digitsBucket[digit-1] > 1){
            return false;       // digit occurs more than one time in the number
        }

        num /= 10;
    }

    // No need to go through the bucket - if we got here, there is one occurance of each digit becuase:
    // 1. The number has 9 digits
    // 2. each digit was found exactly one time (otherwise, false would have been returned)
    // 3. 0 wasnt in the number
    return true;
}

void initBoard(char board[3][3], int size){
    int digit = 1;
    for (size_t row = 0; row < size; row++){
        for (size_t col = 0; col < size; col++){
            board[row][col] = '0' + digit++;
        }
    }
}

void printBoard(char board[3][3], int size){
    for (size_t row = 0; row < size; row++){
        for (size_t col = 0; col < size; col++){
            cout << board[row][col];
            if (col != size - 1){
                cout << ", ";
            }
        }
        cout << endl;
    }
}

// Checks if there is a winner - if not, returns -1, else - returns 1 for AI, 0 for Player
int checkWinner(char board[3][3], int size) {
    // Check rows and columns simultaneously
    for (size_t i = 0; i < size; i++) {
        // Check row i
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
            return board[i][0] == 'X' ? 1 : 0;
        }
        // Check column i
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
            return board[0][i] == 'X' ? 1 : 0;
        }
    }
    // Check main diagonal
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
        return board[0][0] == 'X' ? 1 : 0;
    }
    // Check anti-diagonal
    if (board[2][0] == board[1][1] && board[1][1] == board[0][2]) {
        return board[2][0] == 'X' ? 1 : 0;
    }

    return -1;
}
