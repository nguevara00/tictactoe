#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <cstddef>
#include <vector>

enum Cell {
    EMPTY,
    PLAYER1,
    PLAYER2
};

struct Move {
    int row;
    int column;
};

//A Board is a 4x4 2d vector of cells, where each cell must be marked 0, 1, or 2.
using Board = std::vector<std::vector<Cell>>;

/*
----------
Result contains a value, a row and column for the optimal move, the CPU time used to find the solution,
and the number of nodes expanded.

What a value means is dependent on the specific algorithm.

For minimax, value =  1 if Player 1 can force a win
                   =  0 if neither player can force a win
                   = -1 if Player 2 can force a win

For cutoff search, value is the minimax heuristic evaluation at the root
----------
*/

struct Result {
    int value;
    Move bestMove;
    double cpuTime;
    std::size_t nodesExpanded;
};

//Function prototypes and invariants begin here

/*
----------
checkValid

Precondition: 
    board is a Board object. it may be invalid, this function must verify it. 

Postcondition:
    Function returns true if :

        The board is a completed game (one player has already won) OR
        The board is a valid game in progress OR
        The game is a draw

    Function returns false if :

        Board is not 4x4 OR
        a cell contains an invalid value OR
        The number of marked cells for p1 and p2 would violate turn order OR
        Both players have winning positions marked OR
        The winner's mark count is inconsistent given turn order OR
        Additional moves were marked after a winning move
----------
*/
bool checkValid(const Board& board);





/*
----------
minimax
Precondition: 
    board is a valid Board. 
    The current player is inferred from the marks present on the board.

Postcondition:
    Returns a result containing:
        value =  1 if Player 1 can force a win
              =  0 if neither player can force a win
              = -1 if Player 2 can force a win
        bestMove = the move (row/col) selected by the algorithm
        nodesExpanded = the number of search nodes examined
        cpuTime = the cpu time used in the search

        if multiple moves have the same minimax value, the move with the smallest row number is chosen. If there is a tie, the move with the smallest column number is chosen.

        if board is terminal (one player has won OR the board is full), bestMove returned is {-1,-1}

This function is implemented as a recursive function. Every recursive call receives a valid board and the returned value is always measured from player1 perspective. 
This function uses the checkValid helper function
This function uses the algorithm specified in the pseudocode given in the project description

Design notes:
    since we are timing these operations while using recursion, the three minimax functions may need to operate like this:
        Start a timer, initialize a node counter, call a recursive helper function, get the return from the helper, stop the time. 
----------
*/
Result minimax(const Board& board);





/*
----------
eval
(The static evaluation function h(n) present in informed search | f(n) = g(n) + h(n))

Precondition: 
    board is a valid board
    player = 1 for player1, 2 for player2

Postcondition:
    Returns the number of possible winning sets for player minus the number of possible winning sets for the opponent.

    A winning set is possible for a player if it contains no cell marked by the opponent. The set may contain empty cells and cells already marked by the player.

    The returned value is positive when the board favors the player, negative when it favors the opponent, and zero when they have the same number of winning sets.
----------
*/
int eval(const Board& board, int player);





/*
----------
MinimaxCutOff

Precondition:

    Board is a valid Board
    k is non-negative search depth

Postcondition:
    Searches the game tree to depth k using minimax. The root player is inferred from the number of marks on the board.
    All nonterminal positions at depth k are evaluated from the root player's perspective using eval. 

    Returns a Result struct containing:
        value = the minimax evaluation at the root
        bestMove = the move selected by minimax
        nodesExpanded = the number of search nodes expanded
        cpuTime = the processor time used by the search

    ties are resolved the same way as in the minimax function above
    terminal boards (a winning position already marked) are handled same as above

Design notes:
    since we are timing these operations while using recursion, the three minimax functions may need to operate like this:
        Start a timer, initialize a node counter, call a recursive helper function, get the return from the helper, stop the time. 
----------
*/
Result minimaxCutOff(const Board& board, int k);



/*
Precondidtion:
    the input preconditions are the same as minimaxCutOff

Postcondition:
    Searches the game tree to depth k using alpha-beta pruning.

    returns the same result as minimaxCutOff would return for the same board (may examine fewer nodes)
    ties are resolved in the same way as above
    terminal boards are handled same as above

Alpha is the best value the max root player can guarantee
Beta is the best value that the min opponent can guarantee
Any branch pruned cannot change the minimax value or the selected move 

Design notes:
    since we are timing these operations while using recursion, the three minimax functions may need to operate like this:
        Start a timer, initialize a node counter, call a recursive helper function, get the return from the helper, stop the time. 
----------
*/
Result minimaxAlphaBetaCutOff(const Board& board, int k);

#endif // FUNCTIONS_HPP