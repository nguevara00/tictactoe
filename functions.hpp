#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <iostream>
#include <vector>

/*
----------
A Board is a 2d vector, where each element must be marked 0,1, or 2.
using Board = std::vector<std::vector<int>>;
---------
*/
using Board = std::vector<std::vector<int>>;

/*
----------
A solution contains an int
    this can be of {-1,0,1}
        -1 means P2 can force a win
         0 means either player can force a win
         1 means P1 can force a win
A solution contains a double
    this is the CPU time taken to calculate the solution
A solution contains another int
    this is the number of nodes expanded
----------
*/
struct MinimaxResult {
    int value;
    int row;
    int column;
    double cpuTime;
    int nodesExpanded;
};


MinimaxResult minimax(Board);
int eval(Board b, int p);

MinimaxResult minimaxCutOff(Board b, int k);
MinimaxResult minimaxAlphaBetaCutoff(Board b, int k);

#endif FUNCTIONS_H

