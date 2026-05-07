#include"hex_state.h"
class start_first{
public:
    HexState* state;
    near* nb;
    start_first(HexState* s) {
        state = s;
        nb = new near(s);
    }

    // 距离中心（越近越好）
    inline int center_score(int x, int y) {
        int cx = SIZE / 2 + 1;
        int cy = SIZE / 2 + 1;
        return -(abs(x - cx) + abs(y - cy));
    }

    // 单点评分函数（核心）
    inline int evaluate(int x, int y, int player) {
        int score = 0;

        // 中心优先
        score += 5 * center_score(x, y);

        // 邻居加分（连接）
        if (nb->neighbour_check(x, y, player))
            score += 20;

        // 双桥（核心）
        if (nb->double_bridge(x, y, player))
            score += 50;

        // 阻挡对手
        if (nb->neighbour_check(x, y, -player))
            score += 10;

        return score;
    }

    // 获取候选点（剪枝）
    inline vector<Move> getCandidates();

    // 主函数：返回最佳开局点
    inline Move getBestMove(int player);
};