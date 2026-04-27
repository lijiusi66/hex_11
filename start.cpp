<<<<<<< HEAD
#include"hex_state.h"
class first{
    private:
    public:
    
};
=======
#include "hex_state.h"
#include"start.h"
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>

using namespace std;

struct ScoredMove {
    int x, y;
    int score;
};

vector<Move> start_first::getCandidates() {
        vector<Move> res;

        for (int i = 1; i <= SIZE; i++) {
            for (int j = 1; j <= SIZE; j++) {
                if (state->board[i][j] != 0) continue;

                // 剪枝：只考虑附近点
                if (nb->near_any(i, j)) {
                    res.push_back({i, j});
                }
            }
        }

        // 如果是开局（棋子很少），允许中心
        if (res.empty()) {
            res.push_back({SIZE/2+1, SIZE/2+1});
        }

        return res;
    }

    // 主函数：返回最佳开局点
    Move start_first::getBestMove(int player) {
        vector<Move> moves = getCandidates();
        vector<ScoredMove> scored;

        for (auto &m : moves) {
            int sc = evaluate(m.x, m.y, player);
            scored.push_back({m.x, m.y, sc});
        }

        // 排序（高分优先）
        sort(scored.begin(), scored.end(), [](auto &a, auto &b) {
            return a.score > b.score;
        });

        // 随机选前几个（避免固定死）
        int K = min(3, (int)scored.size());
        int idx = rand() % K;

        return {scored[idx].x, scored[idx].y};
    }
>>>>>>> ff6a46c (init)
