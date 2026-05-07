#ifndef HEX_STATE_H
#define HEX_STATE_H

#include <vector>
using namespace std;

const int SIZE = 11;
const int TOTAL_CELLS = SIZE * SIZE;
const int num_of_near=6;
// 虚拟节点ID
const int VIRTUAL_LEFT = TOTAL_CELLS;    // 玩家1的左边界
const int VIRTUAL_RIGHT = TOTAL_CELLS + 1;  // 玩家1的右边界
const int VIRTUAL_TOP = TOTAL_CELLS;     // 玩家-1的上边界
const int VIRTUAL_BOTTOM = TOTAL_CELLS + 1; // 玩家-1的下边界

struct Move {
    int x, y;
};

// 并查集类
class UnionFind {
private:
    int parent[TOTAL_CELLS + 2];  // +2 用于两个虚拟节点
    int rank_[TOTAL_CELLS + 2];
    
public:
    void init() {
        for (int i = 0; i < TOTAL_CELLS + 2; i++) {
            parent[i] = i;
            rank_[i] = 0;
        }
    }
    
    int find(int x) {
        if (parent[x] != x) {
            parent[x] = find(parent[x]);  // 路径压缩
        }
        return parent[x];
    }
    
    void unite(int x, int y) {
        int px = find(x), py = find(y);
        if (px == py) return;
        // 按秩合并
        if (rank_[px] < rank_[py]) swap(px, py);
        parent[py] = px;
        if (rank_[px] == rank_[py]) rank_[px]++;
    }
    
    bool connected(int x, int y) {
        return find(x) == find(y);
    }
};

class HexState {
public:
    int board[SIZE+2][SIZE+2];
    UnionFind uf[2];  // 0: 玩家1, 1: 玩家-1
    bool uf_initialized[2];

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
        uf_initialized[0] = false;
        uf_initialized[1] = false;
    }
    
    // 坐标转ID
    inline int pos2id(int x, int y) const {
        return (x - 1) * SIZE + (y - 1);
    }
    
    // ID转坐标
    inline void id2pos(int id, int &x, int &y) const {
        x = id / SIZE + 1;
        y = id % SIZE + 1;
    }
    
    // 初始化并查集（用于胜负判断）
    void initUnionFind(int player);
    
    // 检查玩家是否获胜
    bool checkWin(int player);
    
    // 放置棋子并更新并查集
    bool placeAndUpdate(int x, int y, int player);
    
    inline bool loadFromInput(int n);
    inline bool in_board(int x, int y) const {
        return board[x][y] != 2;
    }

    bool place(int x, int y, int player) {
        if (!in_board(x,y) || board[x][y] != 0) return false;
        board[x][y] = player;
        return true;
    }
    HexState(const HexState& other);
    HexState& operator=(const HexState& other);
};

class near {
private:
    HexState* hex_board;
    const int dx[num_of_near] = {0,-1,-1,0,1,1};
    const int dy[num_of_near] = {-1,0,1,1,0,-1};
    const int dx2[num_of_near]={-1,-2,-1,1,2,1};
    const int dy2[num_of_near]={-1,1,2,1,-1,-2};
public:
    near(HexState* b) {
        hex_board = b;
    }
    // 是否有相邻己方棋子
    bool neighbour_check(int x, int y, int player) {
        for (int i = 0; i < num_of_near; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (hex_board->board[nx][ny] == player)
                return true;
        }
        return false;
    }

    // Hex经典：双桥结构检测
    bool double_bridge(int x, int y) {
        /*
            桥结构：
            (x,y) 放下后，如果存在：
            
            X . 
             . X

            两个点形成“不可同时封堵”的连接
        */
        
        int cnt = 0;
        for(int i=0;i<num_of_near;i++){
            int nx = x + dx2[i];
            int ny = y + dy2[i];
            if ((hex_board->board[nx][ny] == 1 ||
                hex_board->board[nx][ny] == -1))return true;
        }
        return false;
    }

    // 是否靠近已有结构（用于剪枝/MCTS）
    bool near_any(int x, int y) {
        for (int i = 0; i < num_of_near; i++) {
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