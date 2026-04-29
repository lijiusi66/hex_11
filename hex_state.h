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
    int board[SIZE+2][SIZE+2];

    HexState() {
        init();
    }

    // 初始化棋盘（边界=2）
    inline void init() {
        for (int i = 0; i < SIZE+2; i++) {
            for (int j = 0; j < SIZE+2; j++) {
                if (i == 0 || j == 0 || i == SIZE+1 || j == SIZE+1)
                    board[i][j] = 2; // 边界
                else
                    board[i][j] = 0; // 空
            }
        }
    }
    inline bool loadFromInput(int n);
    inline bool in_board(int x, int y) const {
        return x >= 1 && x <= SIZE && y >= 1 && y <= SIZE;
    }

    bool place(int x, int y, int player) {
        if (!in_board(x,y) || board[x][y] != 0) return false;
        board[x][y] = player;
        return true;
    }
};

class near {
private:
    HexState* hex_board;

    int dx[6] = {-1,-1,0,0,1,1};
    int dy[6] = {0,1,-1,1,-1,0};

public:
    near(HexState* b) {
        hex_board = b;
    }

    // 是否有相邻己方棋子
    bool neighbour_check(int x, int y, int player) {
        for (int i = 0; i < 6; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (hex_board->board[nx][ny] == player)
                return true;
        }
        return false;
    }

    // Hex经典：双桥结构检测
    bool double_bridge(int x, int y, int player) {
        /*
            桥结构：
            (x,y) 放下后，如果存在：
            
            X . 
             . X

            两个点形成“不可同时封堵”的连接
        */

        int cnt = 0;

        // 所有方向组合
        for (int i = 0; i < 6; i++) {
            int x1 = x + dx[i];
            int y1 = y + dy[i];

            int x2 = x + dx[(i+2)%6];
            int y2 = y + dy[(i+2)%6];

            if (hex_board->board[x1][y1] == player &&
                hex_board->board[x2][y2] == player) {
                cnt++;
            }
        }

        return cnt >= 1;
    }

    // 是否靠近已有结构（用于剪枝/MCTS）
    bool near_any(int x, int y) {
        for (int i = 0; i < 6; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (hex_board->board[nx][ny] == 1 ||
                hex_board->board[nx][ny] == -1)
                return true;
        }
        return false;
    }
};

#endif