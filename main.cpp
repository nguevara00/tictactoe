#include "functions.hpp"

#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

const int WINNING_PATTERNS[24][4] = {
    {0, 1, 2, 3}, {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 2, 3, 1},
    {0, 3, 1, 2}, {0, 3, 2, 1}, {1, 0, 2, 3}, {1, 0, 3, 2},
    {1, 2, 0, 3}, {1, 2, 3, 0}, {1, 3, 0, 2}, {1, 3, 2, 0},
    {2, 0, 1, 3}, {2, 0, 3, 1}, {2, 1, 0, 3}, {2, 1, 3, 0},
    {2, 3, 0, 1}, {2, 3, 1, 0}, {3, 0, 1, 2}, {3, 0, 2, 1},
    {3, 1, 0, 2}, {3, 1, 2, 0}, {3, 2, 0, 1}, {3, 2, 1, 0}
};

enum GameState {
    IN_PROGRESS,
    PLAYER1_WON,
    PLAYER2_WON,
    DRAW
};

bool readInteger(const std::string& prompt, int minimum, int maximum, int& value) {
    while (true) {
        std::cout << prompt;

        if (std::cin >> value) {
            if (value >= minimum && value <= maximum) {
                return true;
            }

            std::cout << "Enter a value from " << minimum << " through "
                      << maximum << ".\n";
        } else {
            if (std::cin.eof()) {
                return false;
            }

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Please enter an integer.\n";
        }
    }
}

bool readDepth(int& depth) {
    return readInteger("Depth k: ", 0, std::numeric_limits<int>::max(), depth);
}

bool readExpected(std::istringstream& input, char expected) {
    char actual;

    if (!(input >> actual)) {
        return false;
    }

    return actual == expected;
}

bool parseBoard(const std::string& text, Board& board) {
    std::istringstream input(text);
    Board parsed(4, std::vector<Cell>(4, EMPTY));

    if (!readExpected(input, '[')) {
        return false;
    }

    for (int row = 0; row < 4; ++row) {
        if (!readExpected(input, '[')) {
            return false;
        }

        for (int column = 0; column < 4; ++column) {
            int value;

            if (!(input >> value) || value < 0 || value > 2) {
                return false;
            }

            parsed[row][column] = static_cast<Cell>(value);

            if (column < 3 && !readExpected(input, ',')) {
                return false;
            }
        }

        if (!readExpected(input, ']')) {
            return false;
        }

        if (row < 3 && !readExpected(input, ',')) {
            return false;
        }
    }

    if (!readExpected(input, ']')) {
        return false;
    }

    input >> std::ws;

    if (input.peek() != std::char_traits<char>::eof()) {
        return false;
    }

    board = parsed;
    return true;
}

bool readBoard(Board& board) {
    while (true) {
        std::string text;

        std::cout << "Enter the board in the form "
                  << "[[1, 0, 0, 0], [0, 1, 0, 2], "
                  << "[0, 0, 0, 0], [0, 0, 0, 0]]:\n";

        if (!std::getline(std::cin >> std::ws, text)) {
            return false;
        }

        if (parseBoard(text, board)) {
            return true;
        }

        std::cout << "Invalid board format. Enter exactly four rows of four "
                  << "values using brackets and commas.\n";
    }
}

bool readValidBoard(Board& board) {
    while (true) {
        if (!readBoard(board)) {
            return false;
        }

        if (checkValid(board)) {
            return true;
        }

        std::cout << "That board is invalid. Please enter another board.\n";
    }
}

bool hasWin(const Board& board, int player) {
    for (int pattern = 0; pattern < 24; ++pattern) {
        bool win = true;

        for (int row = 0; row < 4; ++row) {
            int column = WINNING_PATTERNS[pattern][row];

            if (board[row][column] != static_cast<Cell>(player)) {
                win = false;
                break;
            }
        }

        if (win) {
            return true;
        }
    }

    return false;
}

bool isFull(const Board& board) {
    for (const std::vector<Cell>& row : board) {
        for (Cell cell : row) {
            if (cell == EMPTY) {
                return false;
            }
        }
    }

    return true;
}

GameState getGameState(const Board& board) {
    if (hasWin(board, 1)) {
        return PLAYER1_WON;
    }

    if (hasWin(board, 2)) {
        return PLAYER2_WON;
    }

    if (isFull(board)) {
        return DRAW;
    }

    return IN_PROGRESS;
}

int countMarks(const Board& board, int player) {
    int count = 0;

    for (const std::vector<Cell>& row : board) {
        for (Cell cell : row) {
            if (cell == static_cast<Cell>(player)) {
                ++count;
            }
        }
    }

    return count;
}

int playerToMove(const Board& board) {
    if (countMarks(board, 1) == countMarks(board, 2)) {
        return 1;
    }

    return 2;
}

void displayBoard(const Board& board) {
    std::cout << "\n    1 2 3 4\n";

    for (int row = 0; row < 4; ++row) {
        std::cout << row + 1 << "   ";

        for (int column = 0; column < 4; ++column) {
            std::cout << static_cast<int>(board[row][column]);

            if (column < 3) {
                std::cout << ' ';
            }
        }

        std::cout << '\n';
    }
}

void displayOutcome(GameState state) {
    if (state == PLAYER1_WON) {
        std::cout << "Player 1 wins.\n";
    } else if (state == PLAYER2_WON) {
        std::cout << "Player 2 wins.\n";
    } else if (state == DRAW) {
        std::cout << "The game is a draw.\n";
    }
}

void displayResult(const Result& result) {
    std::cout << "Value: " << result.value << '\n';

    if (result.bestMove.row == -1) {
        std::cout << "Best move: none\n";
    } else {
        std::cout << "Best move: (" << result.bestMove.row + 1 << ", "
                  << result.bestMove.column + 1 << ")\n";
    }

    std::cout << "Nodes expanded: " << result.nodesExpanded << '\n';
    std::cout << "CPU time: " << result.cpuTime << " seconds\n";
}

bool humanTurn(Board& board, int player) {
    while (true) {
        int row;
        int column;

        std::cout << "Player " << player << "'s turn.\n";

        if (!readInteger("Row: ", 1, 4, row)) {
            return false;
        }

        if (!readInteger("Column: ", 1, 4, column)) {
            return false;
        }

        if (board[row - 1][column - 1] != EMPTY) {
            std::cout << "That square is occupied. Try again.\n";
            continue;
        }

        board[row - 1][column - 1] = static_cast<Cell>(player);
        return true;
    }
}

bool computerTurn(Board& board, int player, int depth) {
    std::cout << "Computer is thinking...\n";

    Result result = minimaxAlphaBetaCutOff(board, depth);

    if (result.bestMove.row == -1 || result.bestMove.column == -1) {
        return false;
    }

    board[result.bestMove.row][result.bestMove.column] =
        static_cast<Cell>(player);

    std::cout << "Computer played (" << result.bestMove.row + 1 << ", "
              << result.bestMove.column + 1 << ").\n";

    return true;
}

bool playGame(Board board, int humanPlayer, int depth) {
    while (getGameState(board) == IN_PROGRESS) {
        displayBoard(board);

        int player = playerToMove(board);

        if (player == humanPlayer) {
            if (!humanTurn(board, player)) {
                return false;
            }
        } else if (!computerTurn(board, player, depth)) {
            return false;
        }
    }

    displayBoard(board);
    displayOutcome(getGameState(board));
    return true;
}

void runOptionZero() {
    int humanFirst;
    int depth;

    if (!readInteger("Play first? Enter 1 for yes or 0 for no: ", 0, 1,
                     humanFirst)) {
        return;
    }

    if (!readDepth(depth)) {
        return;
    }

    Board board(4, std::vector<Cell>(4, EMPTY));
    int humanPlayer = humanFirst == 1 ? 1 : 2;
    playGame(board, humanPlayer, depth);
}

void runOptionOne() {
    Board board;

    if (readValidBoard(board)) {
        displayResult(minimax(board));
    }
}

void runOptionTwo() {
    Board board;
    int depth;

    if (!readValidBoard(board)) {
        return;
    }

    if (!readDepth(depth)) {
        return;
    }

    displayResult(minimaxAlphaBetaCutOff(board, depth));
}

void runOptionThree() {
    Board board;
    int depth;

    if (!readValidBoard(board)) {
        return;
    }

    if (!readDepth(depth)) {
        return;
    }

    std::cout << "Minimax cutoff:\n";
    displayResult(minimaxCutOff(board, depth));

    std::cout << "\nAlpha-beta cutoff:\n";
    displayResult(minimaxAlphaBetaCutOff(board, depth));
}

void runOptionFour() {
    Board board;
    int humanPlayer;
    int depth;

    if (!readValidBoard(board)) {
        return;
    }

    GameState state = getGameState(board);

    if (state != IN_PROGRESS) {
        displayBoard(board);
        displayOutcome(state);
        return;
    }

    if (!readInteger("Choose Player 1 or Player 2: ", 1, 2, humanPlayer)) {
        return;
    }

    if (!readDepth(depth)) {
        return;
    }

    playGame(board, humanPlayer, depth);
}

void displayMenu() {
    std::cout << "\n"
              << "0. Play from an empty board\n"
              << "1. Run minimax\n"
              << "2. Run alpha-beta cutoff\n"
              << "3. Compare cutoff algorithms\n"
              << "4. Play from a specified board\n"
              << "5. Exit\n";
}

}

int main() {
    while (true) {
        displayMenu();

        int option;

        if (!readInteger("Choice: ", 0, 5, option)) {
            break;
        }

        if (option == 5) {
            break;
        }

        if (option == 0) {
            runOptionZero();
        } else if (option == 1) {
            runOptionOne();
        } else if (option == 2) {
            runOptionTwo();
        } else if (option == 3) {
            runOptionThree();
        } else if (option == 4) {
            runOptionFour();
        }
    }

    return 0;
}
