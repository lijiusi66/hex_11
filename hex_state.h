#ifndef HEX_STATE_H
#define HEX_STATE_H

#include <vector>
using namespace std;

const int SIZE = 11;

struct Move {
    int x, y;
};

class HexState {
public:
    int board[SIZE][SIZE];

    HexState();

    // 初始化棋盘
    inline void init();

    // 从输入恢复棋盘
    inline void loadFromInput();

    // 判断是否合法
    inline bool in_board(int x, int y) const;

    // 落子
    bool place(int x, int y, int player);
};

#endif