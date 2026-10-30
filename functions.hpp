#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <iostream>
#include <vector>

/*
----------
A Board is a 2d vector, where each element must be marked 0,1, or 2. A Board is always 4x4.
---------
*/
enum Cell {
    EMPTY,
    PLAYER1,
    PLAYER2
};

using Board = std::vector<std::vector<Cell>>;

/*
----------
Result contains a board value, a row and column for the optimal move, the CPU time used to find the solution,
and the number of nodes expanded.

A board value is 1 if player1 can force a win, 0 if no player can force a win, -1 if player2 can force a win.
----------
*/
struct MinimaxResult {
    int value; // {-1,0,1}
    int row; // {0-3}
    int column; // {0-3}
    double cpuTime;
    int nodesExpanded;
};

/*
----------
Input: a board
Output: a result struct
----------
*/
MinimaxResult minimax(Board);

/*
----------
A static evaluation function.
Inputs: a board b and an integer p. (p is which player you are evaluating for | p1 = 1 or p2 = 2) 
Output: an integer result | result = (Number of Ways P1 can win - Number of ways P2 can win)
----------
*/
int eval(Board b, int p);

/*
----------
Inputs: a board and a depth k (higher depths mean smarter moves and more cpu time)
Output: a result struct
----------
*/
MinimaxResult minimaxCutOff(Board b, int k);
MinimaxResult minimaxAlphaBetaCutoff(Board b, int k);

#endif FUNCTIONS_H

