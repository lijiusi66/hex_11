#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <functional>
#include <iostream>
#include <queue>
#include <set>
#include <vector>
#include <random>
#include <chrono>
#include <unistd.h>   // _exit
using namespace std;
float log_[101]={0,0,0.693147,1.09861,1.38629,1.60944,1.79176,1.94591,2.07944,2.19722,2.30259,2.3979,2.48491,2.56495,2.63906,2.70805,2.77259,2.83321,2.89037,2.94444,2.99573,3.04452,3.09104,3.13549,3.17805,3.21888,3.2581,3.29584,3.3322,3.3673,3.4012,3.43399,3.46574,3.49651,3.52636,3.55535,3.58352,3.61092,3.63759,3.66356,3.68888,3.71357,3.73767,3.7612,3.78419,3.80666,3.82864,3.85015,3.8712,3.89182,3.91202,3.93183,3.95124,3.97029,3.98898,4.00733,4.02535,4.04305,4.06044,4.07754,4.09434,4.11087,4.12713,4.14313,4.15888,4.17439,4.18965,4.20469,4.21951,4.23411,4.2485,4.26268,4.27667,4.29046,4.30407,4.31749,4.33073,4.34381,4.35671,4.36945,4.38203,4.39445,4.40672,4.41884,4.43082,4.44265,4.45435,4.46591,4.47734,4.48864,4.49981,4.51086,4.52179,4.5326,4.54329,4.55388,4.56435,4.57471,4.58497,4.59512,4.60517};
// =====================================================================
// 全局棋局/方向常量
// ---------------------------------------------------------------------
// win 的语义：
//   win == 1 : 己方(player=1)是先手，连接“上下”（x 方向，x=1 与 x=SIZE）
//   win == 0 : 己方(player=1)是后手，连接“左右”（y 方向，y=1 与 y=SIZE）
// 对方(player=-1)的目标方向永远与己方互补。
// =====================================================================
int win;
int check_count_of_node=0;
// [新增] 用于控制 MCTS 候选生成的距离限制
// 开局阶段（棋子数 <= 6）设为 3，允许更多探索
// 中局/残局设为 2，聚焦关键区域
int g_limDist = 2;
const int num_of_near=6;
const int SIZE = 11;
const int TOTAL_CELLS = SIZE * SIZE;

// 每个玩家一个 UnionFind，共用两个虚拟边界节点
// （A = “起点”边界，B = “终点”边界；x/y 方向通过 isVerticalPlayer 区分）
const int VIRTUAL_A = TOTAL_CELLS;
const int VIRTUAL_B = TOTAL_CELLS + 1;

struct Move {
  int x, y;
};

// 该玩家的目标方向是否为“上下”（即 x 方向）
//   己方(player=1)先手 -> 连上下
//   对方(player=-1)在己方后手时 -> 也是先手 -> 连上下
inline bool isVerticalPlayer(int player) {
  return (player == 1 && win == 1) || (player == -1 && win == 0);
}

// 每个玩家独享一个 UnionFind 槽位（0 给“连上下”那一方，1 给“连左右”那一方）
inline int ufIdx(int player) {
  return isVerticalPlayer(player) ? 0 : 1;
}

// =====================================================================
// 桥（two-bridge）几何
// ---------------------------------------------------------------------
// Hex 邻居 6 个方向：dx={-1,-1,0,0,1,1} dy={0,1,-1,1,-1,0}
// 桥是“相隔一格、有两个公共空邻居（carrier）”的稳定连接结构。
// BRIDGE_DX/DY 给出 6 个桥方向（从一颗棋子出发可达的桥端）。
// CARRIER1/2 给出该桥的两个 carrier 相对偏移。
// 任何一个 carrier 被对手占了、另一个还空，就必须立即补另一个，否则桥断。
// =====================================================================
const int BRIDGE_DX[6] = {-1, -2, -1,  1,  2,  1};
const int BRIDGE_DY[6] = {-1,  1,  2,  1, -1, -2};
const int CARRIER1_DX[6] = {-1, -1, 0,  1,  1,  0};
const int CARRIER1_DY[6] = { 0,  1,  1,  0,  -1, -1};
const int CARRIER2_DX[6] = { 0, -1,  -1,  0,  1,  1};
const int CARRIER2_DY[6] = {-1,  0,  1,  1, 0, -1};

// 全局共享随机源（mt19937）—— 避免 rand() 周期短、分布差
inline mt19937 &globalRng() {
  static mt19937 rng(
      (unsigned)chrono::steady_clock::now().time_since_epoch().count());
  return rng;
}
class UnionFind {
public:
  // TOTAL_CELLS+2 = 123 < INT16_MAX，所以用 int16_t 即可
  // 拷贝代价减半，对 MCTS 内每个 Node 拷贝意义大
  int16_t parent[TOTAL_CELLS + 2];
  int16_t rank_[TOTAL_CELLS + 2];

  void init() {
    for (int i = 0; i < TOTAL_CELLS + 2; i++) {
      parent[i] = (int16_t)i;
      rank_[i] = 0;
    }
  }

  int find(int x) {
    if (parent[x] != (int16_t)x) {
      parent[x] = (int16_t)find(parent[x]);
    }
    return parent[x];
  }

  void unite(int x, int y) {
    int px = find(x), py = find(y);
    if (px == py)
      return;
    if (rank_[px] < rank_[py])
      swap(px, py);
    parent[py] = (int16_t)px;
    if (rank_[px] == rank_[py])
      rank_[px]++;
  }

  bool connected(int x, int y) { return find(x) == find(y); }
};

class HexState {
public:
  // board 值域：-1（对手） 0（空） 1（我方） 2（边界 padding）
  // 用 int8_t 把每个 HexState 减小 500 多字节，加速 Node/simState 拷贝
  int8_t board[SIZE + 2][SIZE + 2];
  UnionFind uf[2];
  bool uf_initialized[2];

  HexState() { init(); }

  inline void init() {
    memset(board, 0, sizeof(board));
    for (int i = 0; i < 13; i++) {
      board[0][i] = 2;
      board[12][i] = 2;
      board[i][0] = 2;
      board[i][12] = 2;
    }
    uf_initialized[0] = false;
    uf_initialized[1] = false;
  }

  inline int pos2id(int x, int y) const { return (x - 1) * SIZE + (y - 1); }

  inline void id2pos(int id, int &x, int &y) const {
    x = id / SIZE + 1;
    y = id % SIZE + 1;
  }

  inline bool in_board(int x, int y) const { return board[x][y] != 2; }
  inline bool valid(int x, int y) const {
    return x >= 1 && x <= SIZE && y >= 1 && y <= SIZE && board[x][y] == 0;
  }
  bool place(int x, int y, int player) {
    if (!in_board(x, y) || board[x][y] != 0)
      return false;
    board[x][y] = player;
    return true;
  }

  // 读取 botzone 风格的输入：
  //   side = 1 : 己方先手（连上下）
  //   side = 0 : 己方后手（连左右）
  // 返回 false 表示当前回合就是己方的第一手（对方还没下），此时已直接输出落子。
  inline bool loadFromInput(int n, int& side,Move& mv) {
    int x, y;
    bool sideKnown = false;
    side = 0;

    for (int i = 0; i < n - 1; i++) {
      cin >> x >> y;
      if (!sideKnown) {
        // 第一次读到的“对方第一手”决定了己方的先后手
        side = (x == -1) ? 1 : 0;
        sideKnown = true;
      }
      if (x != -1)
        board[x + 1][y + 1] = -1;
      cin >> x >> y;
      if (x != -1)
        board[x + 1][y + 1] = 1;
    }

    cin >> x >> y;
    mv.x=x;
    mv.y=y;
    if (!sideKnown) {
      side = (x == -1) ? 1 : 0;
      sideKnown = true;
    }
    if (x == -1) {
      // 己方是先手且尚未落第一子：固定开局
      cout << 1 << ' ' << 2 << endl;
      return false;
    }
    board[x + 1][y + 1] = -1;
    return true;
  }

  void initUnionFind(int player) {
    const int DX[6] = {-1, -1, 0, 0, 1, 1};
    const int DY[6] = {0, 1, -1, 1, -1, 0};

    int idx = ufIdx(player);
    uf[idx].init();
    uf_initialized[idx] = true;

    const bool vertical = isVerticalPlayer(player);

    for (int x = 1; x <= SIZE; x++) {
      for (int y = 1; y <= SIZE; y++) {
        if (board[x][y] != player)
          continue;
        int id = pos2id(x, y);

        if (vertical) {
          if (x == 1)    uf[idx].unite(id, VIRTUAL_A);
          if (x == SIZE) uf[idx].unite(id, VIRTUAL_B);
        } else {
          if (y == 1)    uf[idx].unite(id, VIRTUAL_A);
          if (y == SIZE) uf[idx].unite(id, VIRTUAL_B);
        }

        for (int d = 0; d < 6; d++) {
          int nx = x + DX[d];
          int ny = y + DY[d];
          if (in_board(nx, ny) && board[nx][ny] == player) {
            uf[idx].unite(id, pos2id(nx, ny));
          }
        }
      }
    }
  }

  bool checkWin(int player) {
    int idx = ufIdx(player);
    if (!uf_initialized[idx]) {
      initUnionFind(player);
    }
    return uf[idx].connected(VIRTUAL_A, VIRTUAL_B);
  }

  bool placeAndUpdate(int x, int y, int player) {
    const int DX[6] = {-1, -1, 0, 0, 1, 1};
    const int DY[6] = {0, 1, -1, 1, -1, 0};

    if (!in_board(x, y) || board[x][y] != 0)
      return false;
    board[x][y] = player;
    int idx = ufIdx(player);
    int id = pos2id(x, y);

    if (!uf_initialized[idx]) {
      initUnionFind(player);
      return true;
    }

    if (isVerticalPlayer(player)) {
      if (x == 1)    uf[idx].unite(id, VIRTUAL_A);
      if (x == SIZE) uf[idx].unite(id, VIRTUAL_B);
    } else {
      if (y == 1)    uf[idx].unite(id, VIRTUAL_A);
      if (y == SIZE) uf[idx].unite(id, VIRTUAL_B);
    }

    for (int d = 0; d < 6; d++) {
      int nx = x + DX[d];
      int ny = y + DY[d];
      if (in_board(nx, ny) && board[nx][ny] == player) {
        uf[idx].unite(id, pos2id(nx, ny));
      }
    }
    return true;
  }
};

// =====================================================================
// 开局/中盘感知：玩家在目标轴上的当前跨度
// ---------------------------------------------------------------------
// vertical 玩家（连上下）的目标轴 = x；horizontal 玩家（连左右）的目标轴 = y。
// axisMin/axisMax/myCount 用于在 candidateScore 里奖励“扩张跨度”的着法，
// 避免开局阶段所有候选都堆在第一颗棋子（强制下的 C2）周围。
// =====================================================================
struct AxisStat {
  int axisMin;
  int axisMax;
  int myCount;
  bool vertical;
};

inline AxisStat computeAxisStat(HexState &state, int player) {
  AxisStat s;
  s.vertical = isVerticalPlayer(player);
  s.axisMin = SIZE + 1;
  s.axisMax = 0;
  s.myCount = 0;
  for (int i = 1; i <= SIZE; i++) {
    for (int j = 1; j <= SIZE; j++) {
      if (state.board[i][j] != player) continue;
      int a = s.vertical ? i : j;
      if (a < s.axisMin) s.axisMin = a;
      if (a > s.axisMax) s.axisMax = a;
      s.myCount++;
    }
  }
  return s;
}

// =====================================================================
// 启发分（用于候选排序、starter 决策、MCTS 内 prior）
// ---------------------------------------------------------------------
// pieces：当前棋盘已落子总数；用来切换“开局/中盘/残局”三档权重。
//   - 开局 pieces < 8：邻接奖励显著降低，中心+跨棋盘部署权重升高；
//                       新增“扩张目标轴跨度”奖励，避免被 C2 锁死。
//   - 中盘/残局：邻接、桥位、封堵权重升高，跨度奖励淡出。
// axis：通过 computeAxisStat 一次性算出，避免每个候选重复 O(SIZE²) 扫描。
// =====================================================================
inline int candidateScore(HexState &state, int x, int y, int player, 
                          int pieces, const AxisStat &axis) {
  static const int dx[6] = {-1, -1, 0, 0, 1, 1};
  static const int dy[6] = {0, 1, -1, 1, -1, 0};
  const int cx = SIZE / 2 + 1, cy = SIZE / 2 + 1;

  const bool earlyGame = pieces < 12;   // MoHex: 前12手算开局
  const bool veryEarly = pieces < 6;    // 前6手算极早期
  int score = 0;
  
  // ========== 1) 中心倾向 ==========
  int centerDist = abs(x - cx) + abs(y - cy);
  score -= (earlyGame ? 10 : 5) * centerDist;

  // ========== 2) 邻接奖励（MoHex 标准权重）==========
  int myAdj = 0, oppAdj = 0;
  for (int d = 0; d < 6; d++) {
    int nx = x + dx[d], ny = y + dy[d];
    if (!state.in_board(nx, ny)) continue;
    if (state.board[nx][ny] == player) myAdj++;
    else if (state.board[nx][ny] == -player) oppAdj++;
  }
  
  // 邻接权重：极早期低（避免扎堆），中盘高
  int adjWeight = veryEarly ? 3 : (earlyGame ? 8 : 15);
  score += adjWeight * myAdj;
  score += (earlyGame ? 5 : 10) * oppAdj;  // 封堵权重

  // ========== 3) 桥位奖励（核心 Hex 战术）==========
  for (int b = 0; b < 6; b++) {
    int bx = x + BRIDGE_DX[b], by = y + BRIDGE_DY[b];
    if (bx < 1 || bx > SIZE || by < 1 || by > SIZE) continue;
    if (state.board[bx][by] != player) continue;
    
    int c1x = x + CARRIER1_DX[b], c1y = y + CARRIER1_DY[b];
    int c2x = x + CARRIER2_DX[b], c2y = y + CARRIER2_DY[b];
    
    // 完整桥（两个carrier都空）vs 半桥
    int bridgeBonus = (state.board[c1x][c1y] == 0 && state.board[c2x][c2y] == 0) 
                      ? 32 : -100;
    score += earlyGame ? (bridgeBonus * 2 / 3) : bridgeBonus;
  }

  // ========== 4) 封堵对方桥（MoHex savebridge 模式）==========
  const int opp = -player;
  for (int b = 0; b < 6; b++) {
    // 检查 (x,y) 是否是对方桥的 carrier
    int s1x = x - CARRIER1_DX[b], s1y = y - CARRIER1_DY[b];
    int p1x = s1x + BRIDGE_DX[b], p1y = s1y + BRIDGE_DY[b];
    int oc2x = s1x + CARRIER2_DX[b], oc2y = s1y + CARRIER2_DY[b];
    
    if (s1x >= 1 && s1x <= SIZE && s1y >= 1 && s1y <= SIZE &&
        p1x >= 1 && p1x <= SIZE && p1y >= 1 && p1y <= SIZE &&
        state.board[s1x][s1y] == opp && state.board[p1x][p1y] == opp) {
      // 这是对方桥的 carrier1，检查 carrier2 是否空
      int blockBonus = (state.board[oc2x][oc2y] == 0) ? 35 : 10;
      score += earlyGame ? (blockBonus * 2 / 3) : blockBonus;
    }
  }

  // ========== 5) 目标轴扩张奖励（MoHex 连接策略）==========
  if (earlyGame && axis.myCount > 0) {
    int a = axis.vertical ? x : y;
    int span = axis.axisMax - axis.axisMin;
    
    // 鼓励突破当前边界
    if (a < axis.axisMin) score += 15 * (axis.axisMin - a);
    else if (a > axis.axisMax) score += 15 * (a - axis.axisMax);
    
    // 极早期额外奖励大跨度
    if (veryEarly && span < 3) {
      if (axis.vertical) {
        if (x > cx) score += 10;  // 向下扩张
        else score += 10;          // 向上扩张
      } else {
        if (y > cy) score += 10;  // 向右扩张
        else score += 10;          // 向左扩张
      }
    }
  }

  // ========== 6) 边界亲和（中后盘）==========
  if (!earlyGame) {
    if (axis.vertical) {
      if (x<3 || x >= SIZE-1) score += 15;
    } else {
      if (y<3 || y >= SIZE-1) score += 15;
    }
  }

  // ========== 7) 虚连接奖励（MoHex VC 启发）==========
  // 如果这手能完成一个虚连接（连接两个己方集团），给予高分
  if (!veryEarly) {
    for (int d = 0; d < 6; d++) {
      int nx = x + dx[d], ny = y + dy[d];
      if (!state.in_board(nx, ny)) continue;
      if (state.board[nx][ny] == player) {
        // 检查是否连接了两个不同的集团
        // 简化版：假设邻接不同位置可能连接不同集团
        score += 5;
      }
    }
  }

  return score;
}


// =====================================================================
// 战术工具（Step 2）
// ---------------------------------------------------------------------
// 三件套：必胜手 / 必应封堵 / 单步桥保护。
// 这些工具同时被 root 决策与 MCTS::generateMoves 调用，
// 让 MCTS 在“必应”位置上做强剪枝（候选只有 1~2 个，搜索深得很多）。
// =====================================================================

// 不拷贝 state，仅借用 UnionFind 当前信息判定：
//   在空格 (x,y) 落 player 一手后，是否立即胜利？
// 思路：胜利条件是 VIRTUAL_A 与 VIRTUAL_B 连通；落子 (x,y) 会把它的
//   6 邻接同色集团 + 自身边界归属合并。检查这一“合并集团”是否同时
//   触达 A 端和 B 端即可，无需真正修改 UF。
//   注：UnionFind.find / connected 会做路径压缩，因此 state 必须非 const。
inline bool wouldWinIfPlace(HexState &state, int x, int y, int player) {
  int idx = ufIdx(player);
  if (!state.uf_initialized[idx]) state.initUnionFind(player);

  const bool vertical = isVerticalPlayer(player);
  bool reachA = (vertical ? (x == 1)    : (y == 1));
  bool reachB = (vertical ? (x == SIZE) : (y == SIZE));
  if (reachA && reachB) return true;

  static const int dx[6] = {-1, -1, 0, 0, 1, 1};
  static const int dy[6] = { 0,  1,-1, 1,-1, 0};
  for (int d = 0; d < 6 && !(reachA && reachB); d++) {
    int nx = x + dx[d], ny = y + dy[d];
    if (nx < 1 || nx > SIZE || ny < 1 || ny > SIZE) continue;
    if (state.board[nx][ny] != player) continue;
    int nid = state.pos2id(nx, ny);
    if (!reachA && state.uf[idx].connected(nid, VIRTUAL_A)) reachA = true;
    if (!reachB && state.uf[idx].connected(nid, VIRTUAL_B)) reachB = true;
  }
  return reachA && reachB;
}

// 我方下哪一步能立即赢？没有则返回 {-1,-1}
inline Move tacticalWin(HexState &state, int player) {
  for (int i = 1; i <= SIZE; i++) {
    for (int j = 1; j <= SIZE; j++) {
      if (state.board[i][j] != 0) continue;
      if (wouldWinIfPlace(state, i, j, player))
        return {i, j};
    }
  }
  return {-1, -1};
}

// 对方下哪一步能立即赢？我方必须封堵那一点
inline Move tacticalBlock(HexState &state, int player) {
  return tacticalWin(state, -player);
}

// 检测我方的桥被对手压到一个 carrier 时，必须立即补另一个 carrier。
// 返回所有“正在被对手攻击、必须立即补救”的 carrier 位置（去重）。
inline vector<Move> bridgeSaves(HexState &state, int player) {
  vector<Move> saves;
  bool taken[SIZE + 2][SIZE + 2];
  memset(taken, 0, sizeof(taken));

  for (int x = 1; x <= SIZE; x++) {
    for (int y = 1; y <= SIZE; y++) {
      if (state.board[x][y] != player) continue;
      for (int b = 0; b < 6; b++) {
        int bx = x + BRIDGE_DX[b];
        int by = y + BRIDGE_DY[b];
        if (bx < 1 || bx > SIZE || by < 1 || by > SIZE) continue;
        if (state.board[bx][by] != player) continue;
        int c1x = x + CARRIER1_DX[b], c1y = y + CARRIER1_DY[b];
        int c2x = x + CARRIER2_DX[b], c2y = y + CARRIER2_DY[b];
        int v1 = state.board[c1x][c1y];
        int v2 = state.board[c2x][c2y];
        if (v1 == -player && v2 == 0 && !taken[c2x][c2y]) {
          saves.push_back({c2x, c2y});
          taken[c2x][c2y] = true;
        } else if (v2 == -player && v1 == 0 && !taken[c1x][c1y]) {
          saves.push_back({c1x, c1y});
          taken[c1x][c1y] = true;
        }
      }
    }
  }
  return saves;
}

// 综合“必应”着法：
//   ① 我方一手赢 → 直接落
//   ② 对方一手赢 → 强制封堵
//   ③ 我方有桥被对手单步攻击 → 立即补 carrier
// 若没有必应，返回空 vector；MCTS 再走完整候选+搜索。
inline vector<Move> mustRespond(HexState &state, int player) {
  Move w = tacticalWin(state, player);
  if (w.x > 0) return {w};

  Move b = tacticalWin(state, -player);
  if (b.x > 0) return {b};

  vector<Move> saves = bridgeSaves(state, player);
  if (!saves.empty()) return saves;

  return {};
}

// =====================================================================
// 轻量桥保护（rollout 用）—— O(36) 而非 O(121 × 36)
// ---------------------------------------------------------------------
// 已知对手上一手落子位置 (lastX, lastY)，仅在其 6 邻己方棋子中查找
// “刚被破坏的桥”，返回需要立即补的另一个 carrier；否则 {-1,-1}。
// =====================================================================
inline Move rolloutBridgeSave(HexState &s, int player, int lastX, int lastY) {
  if (lastX <= 0) return {-1, -1};
  const int dx[6] = {-1, -1, 0, 0, 1, 1};
  const int dy[6] = { 0,  1,-1, 1,-1, 0};
  vector<Move>res;
  Move mv;
  for (int d = 0; d < 6; d++) {
    int ax = lastX + dx[d], ay = lastY + dy[d];
    if (ax < 1 || ax > SIZE || ay < 1 || ay > SIZE) continue;
    if (s.board[ax][ay]!=2&&s.board[ax][ay] != player) continue;
    for (int b = 0; b < 6; b++) {
      int bx = ax + BRIDGE_DX[b], by = ay + BRIDGE_DY[b];
      if (bx < 0 || bx > SIZE+1 || by < 0 || by > SIZE+1) continue;
      if (s.board[bx][by] != 2&&s.board[bx][by] != player) continue;
      int c1x = ax + CARRIER1_DX[b], c1y = ay + CARRIER1_DY[b];
      int c2x = ax + CARRIER2_DX[b], c2y = ay + CARRIER2_DY[b];
      if (c1x == lastX && c1y == lastY && s.board[c2x][c2y] == 0){
        mv.x=c2x;mv.y=c2y;
        res.push_back(mv);
      }
      if (c2x == lastX && c2y == lastY && s.board[c1x][c1y] == 0){
        mv.x=c1x;mv.y=c1y;
        res.push_back(mv);
      }
    }
  }
  if (!res.empty()) {
    uniform_int_distribution<size_t> dist(0, res.size() - 1);
    size_t random_idx = dist(globalRng());
    Move random_move = res[random_idx];
    return random_move;
}
  return {-1, -1};
}
inline Move rolloutBridgeblock(HexState &s, int player, int lastX, int lastY) {
  if (lastX <= 0) return {-1, -1};
  vector<Move>res;
  Move mv;
  for (int d = 0; d < 6; d++) {
    int ax = lastX , ay = lastY;
    for (int b = 0; b < 6; b++) {
      int bx = ax + BRIDGE_DX[b], by = ay + BRIDGE_DY[b];
      if (bx < 0 || bx > SIZE+1 || by < 0 || by > SIZE+1) continue;
      if (s.board[bx][by] != 2&&s.board[bx][by] != -player) continue;
      int c1x = ax + CARRIER1_DX[b], c1y = ay + CARRIER1_DY[b];
      int c2x = ax + CARRIER2_DX[b], c2y = ay + CARRIER2_DY[b];
      if (s.board[c1x][c1y] ==player&& s.board[c2x][c2y] == 0){
        mv.x=c2x;mv.y=c2y;
        return mv;
      }
      if (s.board[c2x][c2y] == player && s.board[c1x][c1y] == 0){
        mv.x=c1x;mv.y=c1y;
        return mv;
      }
    }
  }
  return {-1, -1};
}
class MCTS {
private:
  struct Node {
    HexState state;
    Node *parent;
    vector<Node *> children;
    Move move;
    int player;
    int visits;
    double wins;
    vector<Move> untriedMoves;

    Node(const HexState &s,Move mv) {
      state = s;
      parent = nullptr;
      move = mv;
      player = 0;
      visits = 0;
      wins = 0.0;
    }

    Node(Node *p, Move m, int pl) {
      parent = p;
      move = m;
      player = pl;
      visits = 0;
      wins = 0.0;
      state = p->state;
      state.placeAndUpdate(m.x, m.y, player);
    }

    ~Node() {
      for (auto c : children) {
        delete c;
      }
    }
  };

  Node *root;
  int rootPlayer;

  // UCT 常数：Hex 实践经验 0.7~1.0 比 1.414 更合理（wins 已规一化到 [0,1]）
  static constexpr double C_UCT = 0.4;

  double uct(Node *parent, Node *child) {
    if (child->visits == 0)
      return 1e18;
    return (child->wins / child->visits) +
           C_UCT * sqrt(log((double)parent->visits) / child->visits);
  }

  // Progressive widening：节点访问 N 次后允许 ceil(C * sqrt(N)) 个孩子
  // 让 MCTS 在"宽度 vs 深度"上自适应，避免一开始就铺很多浅孩子
  Node *select(Node *node) {
    const double C_pw = 1.7;
    while (true) {
      int allowed = max(2, min((int)ceil(C_pw * sqrt((double)node->visits + 1.0)),100));
      bool canExpand = !node->untriedMoves.empty() &&
                       (int)node->children.size() < allowed;
      if (canExpand || node->children.empty()) return node;

      double bestValue = -1e18;
      Node *bestChild = nullptr;
      for (auto child : node->children) {
        double v = uct(node, child);
        if (v > bestValue) { bestValue = v; bestChild = child; }
      }
      if (!bestChild) return node;
      node = bestChild;
    }
  }

  Node *expand(Node *node) {
    if (node->untriedMoves.empty())
      return node;

    // untriedMoves 已按 candidateScore 排序、最高分在末尾，pop_back 即可
    Move mv = node->untriedMoves.back();
    node->untriedMoves.pop_back();

    int nextPlayer = (node == root) ? rootPlayer : -node->player;

    Node *child = new Node(node, mv, nextPlayer);
    child->untriedMoves = generateMoves(child->state, -nextPlayer);
    node->children.push_back(child);
    check_count_of_node++;
    return child;
  }

  // Hex 不会和棋；模拟到一方连通即可结束。
  // 在模拟阶段放置棋子并维护空位池
  void placeWithBridge(HexState &simState, vector<int> &empties, int posInEmpty[], int ENC, 
                       int x, int y, int currentPlayer) {
    simState.placeAndUpdate(x, y, currentPlayer);
    int targetCode = x * ENC + y;
    int k = posInEmpty[targetCode];
    int lastCode = empties.back();
    empties[k] = lastCode;
    posInEmpty[lastCode] = k;
    posInEmpty[targetCode] = -1;
    empties.pop_back();
  }

  // 检查并处理桥结构
// 当新下的棋子形成桥时，立即补上另一个承载点
/*const int BRIDGE_DX[6] = {-1, -2, -1,  1,  2,  1};
const int BRIDGE_DY[6] = {-1,  1,  2,  1, -1, -2};
const int CARRIER1_DX[6] = {-1, -1, 0,  1,  1,  0};
const int CARRIER1_DY[6] = { 0,  1,  1,  0,  -1, -1};
const int CARRIER2_DX[6] = { 0, -1,  -1,  0,  1,  1};
const int CARRIER2_DY[6] = {-1,  0,  1,  1, 0, -1};*/
  void checkAndPlaceBridges(HexState &simState, vector<int> &empties, int posInEmpty[], 
                            int ENC, int x, int y, mt19937 &rng) {
    int player = simState.board[x][y];
    
    if((x + 1 <= SIZE && y - 2 >= 1 && simState.board[x][y] == simState.board[x + 1][y - 2])||x+1==SIZE+1||y-2==0) {
      if (simState.board[x + 1][y - 1] == 0 && simState.board[x][y - 1] == 0) {
        if (rng() & 1) {
          placeWithBridge(simState, empties, posInEmpty, ENC, x + 1, y - 1, player);// 先补承载点1
          placeWithBridge(simState, empties, posInEmpty, ENC, x, y - 1, -player);// 再对手补承载点
        } else {
          placeWithBridge(simState, empties, posInEmpty, ENC, x, y - 1, -player);// 先补承载点2
          placeWithBridge(simState, empties, posInEmpty, ENC, x + 1, y - 1, player);// 再补承载点1
        }
      }
    }
    
    if ((x + 1 <= SIZE && y + 1 <= SIZE && simState.board[x][y] == simState.board[x + 1][y + 1])||x+1==SIZE+1||y+1==SIZE+1) {
      if (simState.board[x][y + 1] == 0 && simState.board[x + 1][y] == 0) {
        if (rng() & 1) {
          placeWithBridge(simState, empties, posInEmpty, ENC, x, y + 1, player);
          placeWithBridge(simState, empties, posInEmpty, ENC, x + 1, y, -player);
        } else {
          placeWithBridge(simState, empties, posInEmpty, ENC, x + 1, y, -player);
          placeWithBridge(simState, empties, posInEmpty, ENC, x, y + 1, player);
        }
      }
    }
    
    if ((x + 2 <= SIZE && y - 1 >= 1 && simState.board[x][y] == simState.board[x + 2][y - 1])||x+2==SIZE+1||y-1==0) {
      if (simState.board[x + 1][y - 1] == 0 && simState.board[x + 1][y] == 0) {
        if (rng() & 1) {
          placeWithBridge(simState, empties, posInEmpty, ENC, x + 1, y - 1, player);
          placeWithBridge(simState, empties, posInEmpty, ENC, x + 1, y, -player);
        } else {
          placeWithBridge(simState, empties, posInEmpty, ENC, x + 1, y, -player);
          placeWithBridge(simState, empties, posInEmpty, ENC, x + 1, y - 1, player);
        }
      }
    }
      if ((x-2>=1 && y+1 <= SIZE&& simState.board[x][y] == simState.board[x-2][y + 1])||x-2==0||y+1==SIZE+1) {
        if (simState.board[x - 1][y] == 0 && simState.board[x - 1][y + 1] == 0) {
          if (rng() & 1) {
            placeWithBridge(simState, empties, posInEmpty, ENC, x - 1, y, player);
            placeWithBridge(simState, empties, posInEmpty, ENC, x - 1, y + 1, -player);
          } else {
            placeWithBridge(simState, empties, posInEmpty, ENC, x - 1, y + 1, -player);
            placeWithBridge(simState, empties, posInEmpty, ENC, x - 1, y, player);
          }
        }
      }
      if ((x-1>=1 && y-1>=1&& simState.board[x][y] == simState.board[x - 1][y - 1])||x-1==0||y-1==0) {
        if (simState.board[x - 1][y] == 0 && simState.board[x][y-1] == 0) {
          if (rng() & 1) {
            placeWithBridge(simState, empties, posInEmpty, ENC, x - 1, y, player);
            placeWithBridge(simState, empties, posInEmpty, ENC, x , y - 1, -player);
          } else {
            placeWithBridge(simState, empties, posInEmpty, ENC, x -1, y, -player);
            placeWithBridge(simState, empties, posInEmpty, ENC, x, y-1, player);
          }
        }
      }
    
      if (x -1>=1 && y+2<=SIZE&& simState.board[x][y] == simState.board[x - 1][y +2]) {
        if (simState.board[x][y + 1] == 0 && simState.board[x - 1][y + 1] == 0) {
          if (rng() & 1) {
            placeWithBridge(simState, empties, posInEmpty, ENC, x, y + 1, player);
            placeWithBridge(simState, empties, posInEmpty, ENC, x - 1, y + 1, -player);
          } else {
            placeWithBridge(simState, empties, posInEmpty, ENC, x , y + 1, -player);
            placeWithBridge(simState, empties, posInEmpty, ENC, x-1, y +1, player);
          }
        }
      }
  }

  int simulate(Node *node) {
    HexState simState = node->state;

    if (node != root && simState.checkWin(node->player))
      return node->player;

    int currentPlayer = (node == root) ? rootPlayer : -node->player;

    const int ENC = SIZE + 2;
    static int posInEmpty[(SIZE + 2) * (SIZE + 2)];
    for (int i = 0; i < (SIZE + 2) * (SIZE + 2); i++) posInEmpty[i] = -1;

    vector<int> empties;
    empties.reserve(TOTAL_CELLS);
    for (int i = 1; i <= SIZE; i++) {
      for (int j = 1; j <= SIZE; j++) {
        if (simState.board[i][j] == 0) {
          posInEmpty[i * ENC + j] = (int)empties.size();
          empties.push_back(i * ENC + j);
        }
      }
    }

    int lastX = node->move.x;
    int lastY = node->move.y;

    mt19937 &rng = globalRng();
    int a = uniform_int_distribution<int>(0, 99)(rng);
    while (!empties.empty()) {
      Move mv = {-1, -1};
      if (lastX > 0&&a<75) {
        Move bs = rolloutBridgeSave(simState, currentPlayer, lastX, lastY);
        if (bs.x > 0) mv = bs;
        else mv=rolloutBridgeblock(simState, currentPlayer, lastX, lastY);
      }
      if (mv.x < 0||a>=75) {
        int idx = (int)(rng() % empties.size());
        int code = empties[idx];
        mv = {code / ENC, code % ENC};
      }

      placeWithBridge(simState, empties, posInEmpty, ENC, mv.x, mv.y, currentPlayer);

      if (simState.checkWin(currentPlayer))
        return currentPlayer;

      checkAndPlaceBridges(simState, empties, posInEmpty, ENC, mv.x, mv.y, rng);

      if (simState.checkWin(currentPlayer))
        return currentPlayer;
      if (simState.checkWin(-currentPlayer))
        return -currentPlayer;

      lastX = mv.x;
      lastY = mv.y;
      currentPlayer = -currentPlayer;
    }

    if (simState.checkWin(1))  return 1;
    if (simState.checkWin(-1)) return -1;
    return 0;
  }

  void backpropagate(Node *node, int winner) {
    while (node != nullptr) {
      node->visits++;
      if (winner == 0) {
        // 兜底平局：极小概率发生，作 0.5 处理
        node->wins += 0.5;
      } else if (node->player == winner) {
        node->wins += 1.0;
      }
      node = node->parent;
    }
  }

vector<Move> generateMoves(HexState &state, int player, int widthCap = 20) {
    vector<Move> tactical = mustRespond(state, player);
    if (!tactical.empty()) return tactical;

    int pieces = 0;
    for (int i = 1; i <= SIZE; i++)
      for (int j = 1; j <= SIZE; j++)
        if (state.board[i][j] != 0) pieces++;
    AxisStat axis = computeAxisStat(state, player);

    if (pieces == 0) {
      return { {SIZE / 2 + 1, SIZE / 2 + 1} };
    }

    static const int dx[6] = {-1, -1, 0, 0, 1, 1};
    static const int dy[6] = { 0,  1,-1, 1,-1, 0};

    int dist[SIZE + 2][SIZE + 2];
    memset(dist, -1, sizeof(dist));//初始化为 -1（表示未访问）

    int qx[121], qy[121];// BFS 队列
    int ql = 0, qr = -1;// 队列首尾指针
    // 把所有现有棋子加入队列，距离设为 0
    for (int i = 1; i <= SIZE; i++) {
      for (int j = 1; j <= SIZE; j++) {
        if (state.board[i][j] != 0) {
          dist[i][j] = 0;
          qr++;
          qx[qr] = i;
          qy[qr] = j;
        }
      }
    }
    //开局阶段 （棋子数 ≤ 6）：限制为 3 步 ，允许更多探索
    //中局/残局 ：限制为 2 步 ，聚焦关键区域
    int limDist = g_limDist;
    if (pieces <= 6) limDist = 3;

    while (ql <= qr) {
      int x = qx[ql], y = qy[ql];
      ql++;
      if (dist[x][y] == limDist) continue;// 达到限制，停止扩散
      for (int d = 0; d < 6; d++) {
        int nx = x + dx[d], ny = y + dy[d];
        if (nx < 1 || nx > SIZE || ny < 1 || ny > SIZE) continue;
        if (dist[nx][ny] != -1) continue;
        dist[nx][ny] = dist[x][y] + 1;
        qr++;
        qx[qr] = nx;
        qy[qr] = ny;
      }
    }

    struct Scored { int x, y, s; };
    vector<Scored> scored;
    scored.reserve(64);
// 只收集距离在 [1, limDist] 范围内的空格
    for (int i = 1; i <= SIZE; i++) {
      for (int j = 1; j <= SIZE; j++) {
        if (dist[i][j] > 0 && dist[i][j] <= limDist) {
          // 计算启发分数
          scored.push_back({i, j, candidateScore(state, i, j, player, pieces, axis)});
        }
      }
    }

    if (scored.empty()) {
      for (int i = 1; i <= SIZE; i++) {
        for (int j = 1; j <= SIZE; j++) {
          if (state.board[i][j] == 0)
            scored.push_back({i, j, candidateScore(state, i, j, player, pieces, axis)});
        }
      }
    }

    sort(scored.begin(), scored.end(),
         [](const Scored &a, const Scored &b) { return a.s > b.s; });

    if ((int)scored.size() > widthCap) scored.resize(widthCap);

    reverse(scored.begin(), scored.end());

    vector<Move> moves;
    moves.reserve(scored.size());
    for (auto &s : scored) moves.push_back({s.x, s.y});
    return moves;
}

public:
  MCTS(HexState *state, int player,Move mv) {
    rootPlayer = player;
    root = new Node(*state,mv);
    root->untriedMoves = generateMoves(root->state, rootPlayer, 36);
  }

  ~MCTS() { delete root; }

  Move search(double time_limit_sec) {
    // 用 wall clock 而非 CPU time（clock()）。
    // botzone 计时基于墙钟，若机器有 IO/调度抖动，clock() 会乐观估时间。
    using clk = chrono::steady_clock;
    auto start = clk::now();
    auto budget = chrono::nanoseconds((long long)(time_limit_sec * 1e9));
    auto deadline = start + budget;

    // 每 64 次迭代检一次 deadline，减少 chrono 调用开销
    int iter_since_check = 0;
    while (true) {
      if ((iter_since_check++ & 0x1F) == 0) {
        if (clk::now() >= deadline) break;
      }
      Node *node = root;
      node = select(node);
      node = expand(node);
      int winner = simulate(node);
      backpropagate(node, winner);
    }

    // Robust max：以准确率为主要标准，胜率作为破平局
    Node *bestChild = nullptr;
    int bestVisit = -1;
    double bestRate = -1.0;
    for (auto child : root->children) {
      double rate = (child->visits > 0) ? (child->wins / child->visits) : 0.0;
      if (rate > bestRate) {
        bestVisit = child->visits;
        bestRate = rate;
        bestChild = child;
      }
    }
    if (bestChild == nullptr) {
      // 没有任何 expand（极少见，比如时间太短）：从 untriedMoves 中取启发分最高的
      if (!root->untriedMoves.empty()) return root->untriedMoves.back();
      return {SIZE / 2 + 1, SIZE / 2 + 1};
    }
    return bestChild->move;
  }
};

int countPieces(HexState &state) {
  int cnt = 0;
  for (int i = 1; i <= SIZE; i++) {
    for (int j = 1; j <= SIZE; j++) {
      if (state.board[i][j] != 0)
        cnt++;
    }
  }
  return cnt;
}

int openingBookActions3rd[121] = {
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,80,40,45,80,48,50,45,-1,
  -1,35,73,34,40,40,73,73,45,-1,-1,
  -1,73,80,80,50,80,61,73,80,45,45,
  -1,80,80,45,45,50,42,80,80,50,58,
  40,70,45,59,45,80,80,80,80,72,80,
  84,85,51,45,45,70,80,80,80,51,80,
  59,80,80,51,45,80,70,70,80,72,60,
  50,59,45,69,80,80,80,70,70,80,72,
  42,71,100,80,80,70,96,70,70,80,40,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

int openingBookActions4th[121] = {
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  40,40,-1,40,40,40,28,40,95,51,62,
  71,40,40,40,71,40,96,47,83,62,95,
  95,36,95,95,95,50,95,83,83,40,20,
  40,71,71,71,71,71,71,62,29,40,40,
  92,95,40,71,71,71,40,40,40,96,96,
  40,40,59,71,81,40,40,84,40,96,40,
  40,93,90,-1,70,71,50,50,40,107,40,
  47,40,79,71,71,40,40,84,50,50,40,
  40,40,40,40,40,50,50,40,40,40,40,
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

inline int gid(int x, int y) { return (x - 1) * SIZE + (y - 1); }//坐标转索引
inline void revid(int id, int &x, int &y) { x = id / SIZE + 1; y = id % SIZE + 1; }//索引转坐标

Move openingBook(HexState &state, int player) {
  int pieces = countPieces(state);

  if (pieces == 0)
    return {3, 2};

  if (pieces == 1 && player == -1)
    return {8, 4};

  if (pieces == 2 && player == 1) {
    for (int i = 1; i <= SIZE; i++) {
      for (int j = 1; j <= SIZE; j++) {
        if (state.board[i][j] == -1) {
          int idx = gid(i, j);
          if (openingBookActions3rd[idx] != -1) {
            int x, y;
            revid(openingBookActions3rd[idx], x, y);
            return {x, y};
          }
        }
      }
    }
  }

  if (pieces == 3 && player == -1) {
    for (int i = 1; i <= SIZE; i++) {
      for (int j = 1; j <= SIZE; j++) {
        if (state.board[i][j] == 1 && !(i == 3 && j == 2) && !(i == 8 && j == 4)) {
          int idx = gid(i, j);
          if (openingBookActions4th[idx] != -1) {
            int x, y;
            revid(openingBookActions4th[idx], x, y);
            return {x, y};
          }
        }
      }
    }
  }

  if (player == -1) {
    if (pieces == 1 && state.board[3][2] == 1)
      return {4, 8};

    if (pieces == 3 && state.board[3][2]==1 && state.board[4][8]==-1 && state.board[8][7]==1)
      return {8, 4};

    if (pieces == 3 && state.board[3][2]==1 && state.board[4][8]==-1 && state.board[2][8]==1)
      return {8, 4};

    if (pieces == 3 && state.board[3][2]==1 && state.board[7][8]==-1 && state.board[4][8]==1)
      return {4,5};

    if (pieces == 3 && state.board[3][2]==1 && state.board[6][6]==-1 && state.board[8][7]==1)
      return {8,5};

    if (pieces ==5 && state.board[3][2]==1 && state.board[6][6]==-1 && state.board[8][7]==1
        && state.board[8][5]==-1 && state.board[2][5]==1)
      return {4,7};
  }

  if (player == 1) {
    if (pieces ==2 && state.board[4][8]==-1)
      return {8,7};

    if (pieces ==2 && state.board[7][8]==-1)
      return {4,8};

    if (pieces ==2 && state.board[6][6]==-1)
      return {8,7};

    if (pieces ==4 && state.board[2][8]==1 && state.board[8][4]==-1 && state.board[4][8]==-1)
      return {10,4};

    if (pieces ==4 && state.board[6][6]==-1 && state.board[8][7]==1 && state.board[8][5]==-1)
      return {2,5};

    if (pieces ==6 && state.board[6][6]==-1 && state.board[8][7]==1 && state.board[8][5]==-1
        && state.board[2][5]==1 && state.board[4][7]==-1)
      return {2,8};
  }

  return {-1, -1};
}

extern int g_limDist;

// 输出一手并立刻退出。
[[noreturn]] static void emitAndExit(int bx, int by) {
  cout << bx << ' ' << by << '\n';
  cout.flush();
  _exit(0);
}

int main() {
  using clk = chrono::steady_clock;
  auto t_start = clk::now();

  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  HexState hex;

#ifndef _BOTZONE_ONLINE
  freopen("in.txt", "r", stdin);
#endif

  int n;
  cin >> n;
  Move mv;
  if (!hex.loadFromInput(n, win,mv)) {
    cout.flush();
    _exit(0);
  }
  int current_player = 1;

  vector<Move> tactical = mustRespond(hex, current_player);
  if (!tactical.empty()) {
    Move m = tactical.front();
    emitAndExit(m.x - 1, m.y - 1);
  }

  int pieces = countPieces(hex);

  if (pieces <= 6) {
    g_limDist = 3;
  } else {
    g_limDist = 2;
  }

  if (pieces <= 5) {
    Move bookMove = openingBook(hex, current_player);
    if (bookMove.x > 0) {
        emitAndExit(bookMove.x - 1, bookMove.y - 1);
    }
  }

  constexpr double HARD_LIMIT = 0.93;
  double elapsed = chrono::duration<double>(clk::now() - t_start).count();
  double remaining = HARD_LIMIT - elapsed-0.05;

  double base_budget;
  if (pieces >= 80)      base_budget = 0.85;
  else if (pieces >= 50) base_budget = 0.85;
  else if (pieces >= 30) base_budget = 0.88;
  else                   base_budget = 0.84;
  double time_limit = min(base_budget, remaining);
  if (time_limit < 0.05) time_limit = 0.05;

#ifndef _BOTZONE_ONLINE
  if (const char *env = getenv("HEX_THINK_TIME"))
    time_limit = atof(env);
#endif

  MCTS mcts(&hex, current_player,mv);
  Move best_move = mcts.search(time_limit);

#ifndef _BOTZONE_ONLINE
  cerr << "expanded_nodes=" << check_count_of_node
       << " think=" << time_limit
       << " total_elapsed="
       << chrono::duration<double>(clk::now() - t_start).count() << "s\n";
#endif
  emitAndExit(best_move.x - 1, best_move.y - 1);
}