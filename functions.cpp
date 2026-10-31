#include "functions.hpp"

#include <ctime>
#include <cstdint>

namespace {
    // Each pattern lists the column used in rows 0..3; all 24 permutations of {0,1,2,3}.
    const int WINNING_PATTERNS[24][4] = {
        {0, 1, 2, 3}, {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 2, 3, 1},
        {0, 3, 1, 2}, {0, 3, 2, 1}, {1, 0, 2, 3}, {1, 0, 3, 2},
        {1, 2, 0, 3}, {1, 2, 3, 0}, {1, 3, 0, 2}, {1, 3, 2, 0},
        {2, 0, 1, 3}, {2, 0, 3, 1}, {2, 1, 0, 3}, {2, 1, 3, 0},
        {2, 3, 0, 1}, {2, 3, 1, 0}, {3, 0, 1, 2}, {3, 0, 2, 1},
        {3, 1, 0, 2}, {3, 1, 2, 0}, {3, 2, 0, 1}, {3, 2, 1, 0}
    };

    bool hasWin(const Board& board, Cell player) {
        for (int p = 0; p < 24; ++p) {
            bool win = true;
            for (int row = 0; row < 4; ++row) {
                if (board[row][WINNING_PATTERNS[p][row]] != player) {
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

    int countMarks(const Board& board, Cell player) {
        int count = 0;
        for (const std::vector<Cell>& row : board) {
            for (Cell cell : row) {
                if (cell == player) {
                    ++count;
                }
            }
        }
        return count;
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

    // True if some single mark of the winner, when removed, leaves no win.
    // That mark could have been the final move, so the game ended legally.
    // If every removal still leaves a win, the game already ended earlier
    // and play continued, so the board is invalid.
    bool winCreatedByLastMove(const Board& board, Cell winner) {
        Board copy = board;
        for (int row = 0; row < 4; ++row) {
            for (int column = 0; column < 4; ++column) {
                if (copy[row][column] == winner) {
                    copy[row][column] = EMPTY;
                    bool stillWins = hasWin(copy, winner);
                    copy[row][column] = winner;
                    if (!stillWins) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    // ---------- Helpers for minimax ----------

    // Powers of 3, used to encode a board as one base-3 number (cell i is digit i).
    const std::uint32_t POW3[16] = {
        1, 3, 9, 27, 81, 243, 729, 2187,
        6561, 19683, 59049, 177147, 531441, 1594323, 4782969, 14348907
    };
    const std::uint32_t NUM_STATES = 43046721;  // 3^16
    const std::int8_t UNKNOWN = 2;              // cache marker: board not solved yet

    std::uint32_t encode(const Board& board) {
        std::uint32_t code = 0;
        for (int i = 0; i < 16; ++i) {
            code += static_cast<std::uint32_t>(board[i / 4][i % 4]) * POW3[i];
        }
        return code;
    }

    Cell playerToMove(const Board& board) {
        if (countMarks(board, PLAYER1) == countMarks(board, PLAYER2)) {
            return PLAYER1;
        }
        return PLAYER2;
    }

    // If the game is over, stores its value (1, -1, or 0 for a draw) and returns true.
    bool terminalValue(const Board& board, int& value) {
        if (hasWin(board, PLAYER1)) {
            value = 1;
            return true;
        }
        if (hasWin(board, PLAYER2)) {
            value = -1;
            return true;
        }
        if (isFull(board)) {
            value = 0;
            return true;
        }
        return false;
    }

    // Recursive minimax from Player 1's perspective (Player 1 = MAX, Player 2 = MIN).
    // board is modified in place and restored before returning.
    int minimaxValue(Board& board, std::uint32_t code, Cell toMove,
                     std::vector<std::int8_t>& cache, std::size_t& nodes) {
        if (cache[code] != UNKNOWN) {
            return cache[code];  // already solved this position
        }

        ++nodes;

        int value;
        if (!terminalValue(board, value)) {
            bool maxNode = (toMove == PLAYER1);
            Cell next = maxNode ? PLAYER2 : PLAYER1;
            int bestPossible = maxNode ? 1 : -1;
            value = maxNode ? -2 : 2;  // stands in for -infinity / +infinity

            for (int i = 0; i < 16; ++i) {
                int row = i / 4;
                int column = i % 4;
                if (board[row][column] != EMPTY) {
                    continue;
                }

                board[row][column] = toMove;
                std::uint32_t childCode =
                    code + static_cast<std::uint32_t>(toMove) * POW3[i];
                int childValue = minimaxValue(board, childCode, next, cache, nodes);
                board[row][column] = EMPTY;

                if (maxNode ? childValue > value : childValue < value) {
                    value = childValue;
                }
                if (value == bestPossible) {
                    break;  // can't do better than a forced win
                }
            }
        }

        cache[code] = static_cast<std::int8_t>(value);
        return value;
    }

    // ---------- Helpers for the cutoff searches ----------

    // Score for a finished game. Bigger than any eval() result (eval is within -24..24),
    // and reduced by ply so that faster wins (and slower losses) are preferred.
    const int WIN_SCORE = 1000;

    // If the game is over, stores its score from rootPlayer's perspective and returns true.
    bool terminalScore(const Board& board, Cell rootPlayer, int ply, int& score) {
        Cell opponent = (rootPlayer == PLAYER1) ? PLAYER2 : PLAYER1;

        if (hasWin(board, rootPlayer)) {
            score = WIN_SCORE - ply;
            return true;
        }
        if (hasWin(board, opponent)) {
            score = -(WIN_SCORE - ply);
            return true;
        }
        if (isFull(board)) {
            score = 0;
            return true;
        }
        return false;
    }

    // Recursive depth-limited minimax. rootPlayer is MAX, the opponent is MIN.
    // ply = depth of this node below the root. board is restored before returning.
    int cutoffValue(Board& board, Cell toMove, Cell rootPlayer,
                    int ply, int k, std::size_t& nodes) {
        ++nodes;

        int score;
        if (terminalScore(board, rootPlayer, ply, score)) {
            return score;
        }
        if (ply == k) {
            return eval(board, rootPlayer == PLAYER1 ? 1 : 2);
        }

        bool maxNode = (toMove == rootPlayer);
        Cell next = (toMove == PLAYER1) ? PLAYER2 : PLAYER1;
        int best = maxNode ? -WIN_SCORE - 1 : WIN_SCORE + 1;  // -inf / +inf

        for (int row = 0; row < 4; ++row) {
            for (int column = 0; column < 4; ++column) {
                if (board[row][column] != EMPTY) {
                    continue;
                }

                board[row][column] = toMove;
                int childValue = cutoffValue(board, next, rootPlayer, ply + 1, k, nodes);
                board[row][column] = EMPTY;

                if (maxNode ? childValue > best : childValue < best) {
                    best = childValue;
                }
            }
        }

        return best;
    }

    // Recursive depth-limited minimax with alpha-beta pruning (Algorithm 12.4).
    // rootPlayer is MAX, the opponent is MIN. Values are from rootPlayer's perspective.
    int alphaBetaValue(Board& board, Cell toMove, Cell rootPlayer, int ply, int k,
                       int alpha, int beta, std::size_t& nodes) {
        ++nodes;

        int score;
        if (terminalScore(board, rootPlayer, ply, score)) {
            return score;
        }
        if (ply == k) {
            return eval(board, rootPlayer == PLAYER1 ? 1 : 2);
        }

        Cell next = (toMove == PLAYER1) ? PLAYER2 : PLAYER1;

        if (toMove == rootPlayer) {  // MAX node
            int res = alpha;
            for (int row = 0; row < 4; ++row) {
                for (int column = 0; column < 4; ++column) {
                    if (board[row][column] != EMPTY) {
                        continue;
                    }

                    board[row][column] = toMove;
                    int val = alphaBetaValue(board, next, rootPlayer, ply + 1, k,
                                             res, beta, nodes);
                    board[row][column] = EMPTY;

                    if (val > res) {
                        res = val;
                    }
                    if (res >= beta) {
                        return res;  // beta cutoff: MIN will never allow this
                    }
                }
            }
            return res;
        }

        // MIN node
        int res = beta;
        for (int row = 0; row < 4; ++row) {
            for (int column = 0; column < 4; ++column) {
                if (board[row][column] != EMPTY) {
                    continue;
                }

                board[row][column] = toMove;
                int val = alphaBetaValue(board, next, rootPlayer, ply + 1, k,
                                         alpha, res, nodes);
                board[row][column] = EMPTY;

                if (val < res) {
                    res = val;
                }
                if (res <= alpha) {
                    return res;  // alpha cutoff: MAX already has something better
                }
            }
        }
        return res;
    }
} // namespace

bool checkValid(const Board& board) {
    // Shape and cell values
    if (board.size() != 4) {
        return false;
    }
    for (const std::vector<Cell>& row : board) {
        if (row.size() != 4) {
            return false;
        }
        for (Cell cell : row) {
            if (cell != EMPTY && cell != PLAYER1 && cell != PLAYER2) {
                return false;
            }
        }
    }

    // Turn order: Player 1 moves first
    int ones = countMarks(board, PLAYER1);
    int twos = countMarks(board, PLAYER2);
    if (ones != twos && ones != twos + 1) {
        return false;
    }

    bool p1Wins = hasWin(board, PLAYER1);
    bool p2Wins = hasWin(board, PLAYER2);

    if (p1Wins && p2Wins) {
        return false;
    }

    // The winner must have made the last move
    if (p1Wins) {
        return ones == twos + 1 && winCreatedByLastMove(board, PLAYER1);
    }
    if (p2Wins) {
        return ones == twos && winCreatedByLastMove(board, PLAYER2);
    }

    return true;  // game in progress or a draw
}

Result minimax(const Board& board) {
    std::clock_t start = std::clock();
    Result result{0, {-1, -1}, 0.0, 0};

    if (!checkValid(board)) {
        return result;
    }

    Board work = board;
    std::vector<std::int8_t> cache(NUM_STATES, UNKNOWN);
    std::size_t nodes = 1;  // the root

    int value;
    if (!terminalValue(work, value)) {
        Cell toMove = playerToMove(work);
        bool maxNode = (toMove == PLAYER1);
        Cell next = maxNode ? PLAYER2 : PLAYER1;
        int bestPossible = maxNode ? 1 : -1;
        std::uint32_t code = encode(work);
        value = maxNode ? -2 : 2;

        // Row-major order, and only a strictly better value replaces the
        // current best, so ties go to the smallest row, then smallest column.
        for (int i = 0; i < 16; ++i) {
            int row = i / 4;
            int column = i % 4;
            if (work[row][column] != EMPTY) {
                continue;
            }

            work[row][column] = toMove;
            std::uint32_t childCode =
                code + static_cast<std::uint32_t>(toMove) * POW3[i];
            int childValue = minimaxValue(work, childCode, next, cache, nodes);
            work[row][column] = EMPTY;

            if (maxNode ? childValue > value : childValue < value) {
                value = childValue;
                result.bestMove = {row, column};
            }
            if (value == bestPossible) {
                break;
            }
        }
    }

    result.value = value;
    result.nodesExpanded = nodes;
    result.cpuTime = static_cast<double>(std::clock() - start) / CLOCKS_PER_SEC;
    return result;
}

int eval(const Board& board, int player) {
    Cell me = (player == 1) ? PLAYER1 : PLAYER2;
    Cell opponent = (player == 1) ? PLAYER2 : PLAYER1;

    int myWays = 0;
    int opponentWays = 0;

    for (int p = 0; p < 24; ++p) {
        bool openForMe = true;        // no opponent mark in this pattern
        bool openForOpponent = true;  // no mark of mine in this pattern

        for (int row = 0; row < 4; ++row) {
            Cell cell = board[row][WINNING_PATTERNS[p][row]];
            if (cell == opponent) {
                openForMe = false;
            } else if (cell == me) {
                openForOpponent = false;
            }
        }

        if (openForMe) {
            ++myWays;
        }
        if (openForOpponent) {
            ++opponentWays;
        }
    }

    return myWays - opponentWays;
}

Result minimaxCutOff(const Board& board, int k) {
    std::clock_t start = std::clock();
    Result result{0, {-1, -1}, 0.0, 0};

    if (!checkValid(board)) {
        return result;
    }

    Board work = board;
    Cell rootPlayer = playerToMove(work);
    Cell next = (rootPlayer == PLAYER1) ? PLAYER2 : PLAYER1;
    std::size_t nodes = 1;  // the root

    int value;
    if (terminalScore(work, rootPlayer, 0, value)) {
        // Game already over: value is set, bestMove stays {-1, -1}
    } else if (k <= 0) {
        // No lookahead: just evaluate the current board
        value = eval(work, rootPlayer == PLAYER1 ? 1 : 2);
    } else {
        value = -WIN_SCORE - 1;

        for (int row = 0; row < 4; ++row) {
            for (int column = 0; column < 4; ++column) {
                if (work[row][column] != EMPTY) {
                    continue;
                }

                work[row][column] = rootPlayer;
                int childValue = cutoffValue(work, next, rootPlayer, 1, k, nodes);
                work[row][column] = EMPTY;

                if (childValue > value) {  // strict: earlier move wins ties
                    value = childValue;
                    result.bestMove = {row, column};
                }
            }
        }
    }

    result.value = value;
    result.nodesExpanded = nodes;
    result.cpuTime = static_cast<double>(std::clock() - start) / CLOCKS_PER_SEC;
    return result;
}

Result minimaxAlphaBetaCutOff(const Board& board, int k) {
    std::clock_t start = std::clock();
    Result result{0, {-1, -1}, 0.0, 0};

    if (!checkValid(board)) {
        return result;
    }

    Board work = board;
    Cell rootPlayer = playerToMove(work);
    Cell next = (rootPlayer == PLAYER1) ? PLAYER2 : PLAYER1;
    std::size_t nodes = 1;  // the root

    int value;
    if (terminalScore(work, rootPlayer, 0, value)) {
        // Game already over: value is set, bestMove stays {-1, -1}
    } else if (k <= 0) {
        // No lookahead: just evaluate the current board
        value = eval(work, rootPlayer == PLAYER1 ? 1 : 2);
    } else {
        const int beta = WIN_SCORE + 1;  // +infinity
        value = -WIN_SCORE - 1;          // -infinity; also serves as alpha

        for (int row = 0; row < 4; ++row) {
            for (int column = 0; column < 4; ++column) {
                if (work[row][column] != EMPTY) {
                    continue;
                }

                work[row][column] = rootPlayer;
                int childValue = alphaBetaValue(work, next, rootPlayer, 1, k,
                                                value, beta, nodes);
                work[row][column] = EMPTY;

                if (childValue > value) {  // strict: earlier move wins ties
                    value = childValue;
                    result.bestMove = {row, column};
                }
            }
        }
    }

    result.value = value;
    result.nodesExpanded = nodes;
    result.cpuTime = static_cast<double>(std::clock() - start) / CLOCKS_PER_SEC;
    return result;
}
