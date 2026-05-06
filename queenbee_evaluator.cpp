#include "queenbee_evaluator.h"
#include <queue>
#include <limits>
#include <algorithm>

QueenbeeEvaluator::QueenbeeEvaluator(HexState* s)
    : state(s), size(SIZE) {
    const int INF = std::numeric_limits<int>::max() / 4;
    for (int p = 0; p < 2; p++) {
        for (int i = 1; i <= size; i++) {
            for (int j = 1; j <= size; j++) {
                distA[p][i][j] = INF;
                distB[p][i][j] = INF;
            }
        }
    }
}

bool QueenbeeEvaluator::isBoundary(int x, int y, int player) const {
    if (player == 1) {
        return x == 1 || x == size;
    } else {
        return y == 1 || y == size;
    }
}

void QueenbeeEvaluator::computeDistances(int player, int resA[SIZE+2][SIZE+2], int resB[SIZE+2][SIZE+2]) {
    const int INF = std::numeric_limits<int>::max() / 4;
    static const int dx[6] = {-1, -1, 0, 0, 1, 1};
    static const int dy[6] = {0, 1, -1, 1, -1, 0};

    for (int i = 1; i <= size; i++) {
        for (int j = 1; j <= size; j++) {
            resA[i][j] = INF;
            resB[i][j] = INF;
        }
    }

    auto pushBoundary = [&](bool startA, std::queue<std::pair<int,int>>& q, int res[SIZE+2][SIZE+2]) {
        for (int i = 1; i <= size; i++) {
            for (int j = 1; j <= size; j++) {
                if (state->board[i][j] == -player) continue;
                if (startA) {
                    if ((player == 1 && i == 1) || (player == -1 && j == 1)) {
                        res[i][j] = 0;
                        q.push({i, j});
                    }
                } else {
                    if ((player == 1 && i == size) || (player == -1 && j == size)) {
                        res[i][j] = 0;
                        q.push({i, j});
                    }
                }
            }
        }
    };

    std::queue<std::pair<int,int>> queueA;
    pushBoundary(true, queueA, resA);
    while (!queueA.empty()) {
        std::pair<int,int> cur = queueA.front();
        queueA.pop();
        int x = cur.first;
        int y = cur.second;
        for (int d = 0; d < 6; d++) {
            int nx = x + dx[d];
            int ny = y + dy[d];
            if (!state->in_board(nx, ny)) continue;
            if (state->board[nx][ny] == -player) continue;
            if (resA[nx][ny] > resA[x][y] + 1) {
                resA[nx][ny] = resA[x][y] + 1;
                queueA.push({nx, ny});
            }
        }
    }

    std::queue<std::pair<int,int>> queueB;
    pushBoundary(false, queueB, resB);
    while (!queueB.empty()) {
        std::pair<int,int> cur = queueB.front();
        queueB.pop();
        int x = cur.first;
        int y = cur.second;
        for (int d = 0; d < 6; d++) {
            int nx = x + dx[d];
            int ny = y + dy[d];
            if (!state->in_board(nx, ny)) continue;
            if (state->board[nx][ny] == -player) continue;
            if (resB[nx][ny] > resB[x][y] + 1) {
                resB[nx][ny] = resB[x][y] + 1;
                queueB.push({nx, ny});
            }
        }
    }
}

int QueenbeeEvaluator::getQueenbeePotential(int player) {
    int idx = playerIndex(player);
    computeDistances(player, distA[idx], distB[idx]);
    const int INF = std::numeric_limits<int>::max() / 4;

    int minPotential = INF;
    for (int i = 1; i <= size; i++) {
        for (int j = 1; j <= size; j++) {
            if (state->board[i][j] != 0) continue;
            int dA = distA[idx][i][j];
            int dB = distB[idx][i][j];
            if (dA >= INF || dB >= INF) continue;
            minPotential = std::min(minPotential, dA + dB);
        }
    }

    return minPotential < INF ? minPotential : INF;
}

int QueenbeeEvaluator::evaluate(HexState* s, int player) {
    state = s;
    const int INF = std::numeric_limits<int>::max() / 4;

    int myPotential = getQueenbeePotential(player);
    int oppPotential = getQueenbeePotential(-player);
    if (myPotential >= INF) myPotential = INF / 2;
    if (oppPotential >= INF) oppPotential = INF / 2;

    int idxMy = playerIndex(player);
    int idxOpp = playerIndex(-player);

    computeDistances(player, distA[idxMy], distB[idxMy]);
    int minMy = INF;
    int mobilityMy = 0;
    for (int i = 1; i <= size; i++) {
        for (int j = 1; j <= size; j++) {
            if (state->board[i][j] != 0) continue;
            int d = distA[idxMy][i][j] + distB[idxMy][i][j];
            if (d >= INF) continue;
            if (d < minMy) {
                minMy = d;
                mobilityMy = 1;
            } else if (d == minMy) {
                mobilityMy++;
            }
        }
    }

    computeDistances(-player, distA[idxOpp], distB[idxOpp]);
    int minOpp = INF;
    int mobilityOpp = 0;
    for (int i = 1; i <= size; i++) {
        for (int j = 1; j <= size; j++) {
            if (state->board[i][j] != 0) continue;
            int d = distA[idxOpp][i][j] + distB[idxOpp][i][j];
            if (d >= INF) continue;
            if (d < minOpp) {
                minOpp = d;
                mobilityOpp = 1;
            } else if (d == minOpp) {
                mobilityOpp++;
            }
        }
    }
    int score = (oppPotential - myPotential) * 100 + (mobilityMy - mobilityOpp);
    return score;
}
