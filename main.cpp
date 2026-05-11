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
using namespace std;
int win;//win作为全局，用于判断是否获胜，先手连接上下，后手连接左右
int check_count_of_node=0;
const int num_of_near=6;
const int SIZE = 11;
const int TOTAL_CELLS = SIZE * SIZE;

const int VIRTUAL_LEFT = TOTAL_CELLS;
const int VIRTUAL_RIGHT = TOTAL_CELLS + 1;
const int VIRTUAL_TOP = TOTAL_CELLS;
const int VIRTUAL_BOTTOM = TOTAL_CELLS + 1;

struct Move {
  int x, y;
};
class UnionFind {
public:
  int parent[TOTAL_CELLS + 2];
  int rank_[TOTAL_CELLS + 2];

  void init() {
    for (int i = 0; i < TOTAL_CELLS + 2; i++) {
      parent[i] = i;
      rank_[i] = 0;
    }
  }

  int find(int x) {
    if (parent[x] != x) {
      parent[x] = find(parent[x]);
    }
    return parent[x];
  }

  void unite(int x, int y) {
    int px = find(x), py = find(y);
    if (px == py)
      return;
    if (rank_[px] < rank_[py])
      swap(px, py);
    parent[py] = px;
    if (rank_[px] == rank_[py])
      rank_[px]++;
  }

  bool connected(int x, int y) { return find(x) == find(y); }
};

class HexState {
public:
  int board[SIZE + 2][SIZE + 2];
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
  inline bool valid(int x, int y) const { return x>=0&&x<=SIZE&&y>=0&&y<=SIZE&&board[x][y] == 0; }
  bool place(int x, int y, int player) {
    if (!in_board(x, y) || board[x][y] != 0)
      return false;
    board[x][y] = player;
    return true;
  }

  inline bool loadFromInput(int n,int& current_player) {
    int x, y;
    current_player=1;
    for (int i = 0; i < n - 1; i++) {
      cin >> x >> y;
      if (x != -1)
        board[x + 1][y + 1] = -1;
        else current_player=0;
      cin >> x >> y;
      if (x != -1)
        board[x + 1][y + 1] = 1;
    }
    cin >> x >> y;
    if (x != -1)
      board[x + 1][y + 1] = -1;
    else {
      cout << 1 << ' ' << 2 << endl;
      return false;
    }
    return true;
  }

  void initUnionFind(int player) {
    const int DX[6] = {-1, -1, 0, 0, 1, 1};
    const int DY[6] = {0, 1, -1, 1, -1, 0};

    int idx = (player == 1) ? win : 1-win;
    uf[idx].init();
    uf_initialized[idx] = true;

    for (int x = 1; x <= SIZE; x++) {
      for (int y = 1; y <= SIZE; y++) {
        if (board[x][y] != player)
          continue;
        int id = pos2id(x, y);

        if (idx==1) {
          if (y == 1)
            uf[idx].unite(id, VIRTUAL_LEFT);
          if (y == SIZE)
            uf[idx].unite(id, VIRTUAL_RIGHT);
        } else {
          if (x == 1)
            uf[idx].unite(id, VIRTUAL_TOP);
          if (x == SIZE)
            uf[idx].unite(id, VIRTUAL_BOTTOM);
        }

        for (int d = 0; d < 6; d++) {
          int nx = x + DX[d];
          int ny = y + DY[d];
          if (in_board(nx, ny) && board[nx][ny] == player) {
            int nid = pos2id(nx, ny);
            uf[idx].unite(id, nid);
          }
        }
      }
    }
  }

  bool checkWin(int player) {
    int idx = (player == 1) ? win : 1-win;
    if (!uf_initialized[idx]) {
      initUnionFind(player);
    }
    if (player == 1) {
      return uf[idx].connected(VIRTUAL_TOP, VIRTUAL_BOTTOM);
    } else {
      return uf[idx].connected(VIRTUAL_LEFT, VIRTUAL_RIGHT);
    }
  }

  bool placeAndUpdate(int x, int y, int player) {
    const int DX[6] = {-1, -1, 0, 0, 1, 1};
    const int DY[6] = {0, 1, -1, 1, -1, 0};

    if (!in_board(x, y) || board[x][y] != 0)
      return false;
    board[x][y] = player;//就是表示当前落子
    int idx = (player == 1) ? win:1-win;
    int id = pos2id(x, y);

    if (!uf_initialized[idx]) {
      initUnionFind(player);
      return true;
    }

    if (player == 1) {
      if (y == 1)
        uf[idx].unite(id, VIRTUAL_TOP);
      if (y == SIZE)
        uf[idx].unite(id, VIRTUAL_BOTTOM);
    } else {
      if (x == 1)
        uf[idx].unite(id, VIRTUAL_LEFT);
      if (x == SIZE)
        uf[idx].unite(id, VIRTUAL_RIGHT);
    }

    for (int d = 0; d < 6; d++) {
      int nx = x + DX[d];
      int ny = y + DY[d];
      if (in_board(nx, ny) && board[nx][ny] == player) {
        int nid = pos2id(nx, ny);
        uf[idx].unite(id, nid);
      }
    }
    return true;
  }
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
      bool double_bridge_fuzzy(int x, int y) {
          /*
              桥结构：
              (x,y) 放下后，如果存在：
              
              X . 
               . X
  
              两个点形成“不可同时封堵”的连接
          */
          
          for(int i=0;i<num_of_near;i++){
              int nx = x + dx2[i];
              int ny = y + dy2[i];
              if (nx>=0&&nx<=SIZE+1&&ny>=0&&ny<=SIZE+1&&(hex_board->board[nx][ny] == 1 ||hex_board->board[nx][ny] == -1))return true;
          }
          return false;
      }
      bool double_bridge(int x, int y,int player){
          for(int i=0;i<num_of_near;i++){
              int nx = x + dx2[i];
              int ny = y + dy2[i];
              if ((hex_board->board[nx][ny] == 1 ||
                  hex_board->board[nx][ny] == -1)&&hex_board->board[x+dx[i]][y+dy[i]]!=!player&&hex_board->board[x+dx[(i+1)%num_of_near]][y+dy[(i+1)%num_of_near]]!=!player)return true;
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
  
class start_first {
public:
  HexState *state;
  near *nb;
  start_first(HexState *s) {
    state = s;
    nb = new near(s);
  }

  inline int center_score(int x, int y) {
    int cx = SIZE / 2 + 1;
    int cy = SIZE / 2 + 1;
    return -(abs(x - cx) + abs(y - cy));
  }

  inline int evaluate(int x, int y, int player) {
    int score = 0;
    score += 5 * center_score(x, y);
    if (nb->neighbour_check(x, y, player))
      score += 20;
    if (nb->double_bridge(x, y, player))
      score += 50;
    if (nb->neighbour_check(x, y, -player))
      score += 10;
    return score;
  }

  vector<Move> getCandidates() {
    vector<Move> res;
    for (int i = 1; i <= SIZE; i++) {
      for (int j = 1; j <= SIZE; j++) {
        if (state->board[i][j] != 0)
          continue;
        if (nb->near_any(i, j)) {
          res.push_back({i, j});
        }
      }
    }
    if (res.empty()) {
      res.push_back({SIZE / 2 + 1, SIZE / 2 + 1});
    }
    return res;
  }

  Move getBestMove(int player) {
    vector<Move> moves = getCandidates();
    struct ScoredMove {
      int x, y, score;
    };
    vector<ScoredMove> scored;
    for (auto &m : moves) {
      int sc = evaluate(m.x, m.y, player);
      scored.push_back({m.x, m.y, sc});
    }
    sort(scored.begin(), scored.end(),
         [](const ScoredMove &a, const ScoredMove &b) {
           return a.score > b.score;
         });
    int K = min(3, (int)scored.size());
    int idx = rand() % K;
    return {scored[idx].x, scored[idx].y};
  }
};

class QueenbeeEvaluator {
private:
  HexState *state;
  int size;
  int distA[2][SIZE + 2][SIZE + 2];
  int distB[2][SIZE + 2][SIZE + 2];

  inline int playerIndex(int player) { return (player == 1) ? 0 : 1; }

public:
  /// Shortest-path distances from both sides for `player` (1 = vertical, -1 = horizontal).
  void computeDistances(int player, int resA[SIZE + 2][SIZE + 2],
                        int resB[SIZE + 2][SIZE + 2]) {
    const int INF = INT_MAX / 4;
    static const int dx[6] = {-1, -1, 0, 0, 1, 1};
    static const int dy[6] = {0, 1, -1, 1, -1, 0};

    for (int i = 1; i <= size; i++) {
      for (int j = 1; j <= size; j++) {
        resA[i][j] = INF;
        resB[i][j] = INF;
      }
    }

    auto pushBoundary = [&](bool startA, queue<pair<int, int>> &q,
                            int res[SIZE + 2][SIZE + 2]) {
      for (int i = 1; i <= size; i++) {
        for (int j = 1; j <= size; j++) {
          if (state->board[i][j] == -player)
            continue;
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

    queue<pair<int, int>> queueA;
    pushBoundary(true, queueA, resA);
    while (!queueA.empty()) {
      auto cur = queueA.front();
      queueA.pop();
      int x = cur.first, y = cur.second;
      for (int d = 0; d < 6; d++) {
        int nx = x + dx[d];
        int ny = y + dy[d];
        if (!state->in_board(nx, ny))
          continue;
        if (state->board[nx][ny] == -player)
          continue;
        if (resA[nx][ny] > resA[x][y] + 1) {
          resA[nx][ny] = resA[x][y] + 1;
          queueA.push({nx, ny});
        }
      }
    }

    queue<pair<int, int>> queueB;
    pushBoundary(false, queueB, resB);
    while (!queueB.empty()) {
      auto cur = queueB.front();
      queueB.pop();
      int x = cur.first, y = cur.second;
      for (int d = 0; d < 6; d++) {
        int nx = x + dx[d];
        int ny = y + dy[d];
        if (!state->in_board(nx, ny))
          continue;
        if (state->board[nx][ny] == -player)
          continue;
        if (resB[nx][ny] > resB[x][y] + 1) {
          resB[nx][ny] = resB[x][y] + 1;
          queueB.push({nx, ny});
        }
      }
    }
  }

private:
  int getQueenbeePotential(int player) {
    int idx = playerIndex(player);
    computeDistances(player, distA[idx], distB[idx]);
    const int INF = INT_MAX / 4;

    int minPotential = INF;
    for (int i = 1; i <= size; i++) {
      for (int j = 1; j <= size; j++) {
        if (state->board[i][j] != 0)
          continue;
        int dA = distA[idx][i][j];
        int dB = distB[idx][i][j];
        if (dA >= INF || dB >= INF)
          continue;
        minPotential = min(minPotential, dA + dB);
      }
    }
    return minPotential < INF ? minPotential : INF;
  }

public:
  QueenbeeEvaluator(HexState *s) : state(s), size(SIZE) {
    const int INF = INT_MAX / 4;
    for (int p = 0; p < 2; p++) {
      for (int i = 1; i <= size; i++) {
        for (int j = 1; j <= size; j++) {
          distA[p][i][j] = INF;
          distB[p][i][j] = INF;
        }
      }
    }
  }

  int evaluate(HexState *s, int player) {
    state = s;
    const int INF = INT_MAX / 4;

    int myPotential = getQueenbeePotential(player);
    int oppPotential = getQueenbeePotential(-player);
    if (myPotential >= INF)
      myPotential = INF / 2;
    if (oppPotential >= INF)
      oppPotential = INF / 2;

    int idxMy = playerIndex(player);
    int idxOpp = playerIndex(-player);

    computeDistances(player, distA[idxMy], distB[idxMy]);
    int minMy = INF;
    int mobilityMy = 0;
    for (int i = 1; i <= size; i++) {
      for (int j = 1; j <= size; j++) {
        if (state->board[i][j] != 0)
          continue;
        int d = distA[idxMy][i][j] + distB[idxMy][i][j];
        if (d >= INF)
          continue;
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
        if (state->board[i][j] != 0)
          continue;
        int d = distA[idxOpp][i][j] + distB[idxOpp][i][j];
        if (d >= INF)
          continue;
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
};

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

    Node(const HexState &s) {
      state = s;
      parent = nullptr;
      move = {-1, -1};
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

  double uct(Node *parent, Node *child) {
    if (child->visits == 0)
      return 1e18;
    return (child->wins / child->visits) +
           1.414 * sqrt(log(parent->visits) / child->visits);
  }

  Node *select(Node *node) {
    while (node->untriedMoves.empty() && !node->children.empty()) {
      double bestValue = -1e18;
      Node *bestChild = nullptr;
      for (auto child : node->children) {
        double uctVal = uct(node, child);
        if (uctVal > bestValue) {
          bestValue = uctVal;
          bestChild = child;
        }
      }
      node = bestChild;
    }
    return node;
  }

  Node *expand(Node *node) {
    if (node->untriedMoves.empty())
      return node;

    int idx = rand() % node->untriedMoves.size();
    Move mv = node->untriedMoves[idx];
    node->untriedMoves.erase(node->untriedMoves.begin() + idx);

    int nextPlayer = -node->player;
    if (node == root)
      nextPlayer = rootPlayer;

    Node *child = new Node(node, mv, nextPlayer);
    child->untriedMoves = generateMoves(child->state, -nextPlayer);
    node->children.push_back(child);
    check_count_of_node++;
    return child;
  }

  int simulate(Node *node) {
    HexState simState = node->state;

    int currentPlayer = -node->player;
    if (node == root)
      currentPlayer = rootPlayer;

    while (true) {
      if (simState.checkWin(currentPlayer))//如果进入这个if分支，说明当前执棋者胜利
        return currentPlayer;

      vector<Move> moves = generateMoves(simState, currentPlayer);
      if (moves.empty())
        break;

      Move mv = moves[rand() % moves.size()];
      simState.placeAndUpdate(mv.x, mv.y, currentPlayer);
      currentPlayer = -currentPlayer;
    }
    if (simState.checkWin(1))
      return 1;
    if (simState.checkWin(-1))
      return -1;
    return -1;
  }

  void backpropagate(Node *node, int winner) {
    while (node != nullptr) {
      node->visits++;
      if (node->player == winner)
        node->wins += 1.0;
      node = node->parent;
    }
  }
vector<Move> generateMoves(HexState &state, int player) {
    vector<Move> moves;

    bool seen[SIZE + 2][SIZE + 2];
    memset(seen, 0, sizeof(seen));

    const int dx[6] = {-1, -1, 0, 0, 1, 1};
    const int dy[6] = {0, 1, -1, 1, -1, 0};

    // Hex 双桥相关偏移
    const int bx[6] = {-2, -1, 1, 2, 1, -1};
    const int by[6] = {1, 2, 1, -1, -2, -1};

    auto pushMove = [&](int x, int y) {
        if (!state.valid(x, y)) return;
        if (state.board[x][y] != 0) return;
        if (seen[x][y]) return;

        seen[x][y] = true;
        moves.push_back({x, y});
    };

    bool emptyBoard = true;

    for (int i = 1; i <= SIZE && emptyBoard; i++) {
        for (int j = 1; j <= SIZE; j++) {
            if (state.board[i][j] != 0) {
                emptyBoard = false;
                break;
            }
        }
    }

    // 开局中心
    if (emptyBoard) {
        pushMove(SIZE / 2 + 1, SIZE / 2 + 1);
        return moves;
    }

    for (int i = 1; i <= SIZE; i++) {
        for (int j = 1; j <= SIZE; j++) {

            if (state.board[i][j] == 0)
                continue;

            // 1. 普通六邻域
            for (int d = 0; d < 6; d++) {
                int nx = i + dx[d];
                int ny = j + dy[d];

                pushMove(nx, ny);
            }

            // 2. 双桥邻域
            for (int d = 0; d < 6; d++) {
                int nx = i + bx[d];
                int ny = j + by[d];

                pushMove(nx, ny);
            }
        }
    }

    // fallback
    if (moves.empty()) {
        for (int i = 1; i <= SIZE; i++) {
            for (int j = 1; j <= SIZE; j++) {
                if (state.board[i][j] == 0)
                    moves.push_back({i, j});
            }
        }
    }

    static mt19937 rng(
        chrono::steady_clock::now().time_since_epoch().count()
    );

    shuffle(moves.begin(), moves.end(), rng);

    return moves;
}

public:
  MCTS(HexState *state, int player) {
    rootPlayer = player;
    root = new Node(*state);
    root->untriedMoves = generateMoves(root->state, rootPlayer);
    srand(time(0));
  }

  ~MCTS() { delete root; }

  Move search(double time_limit_sec) {
    clock_t start_time = clock();
    clock_t limit = start_time + time_limit_sec * CLOCKS_PER_SEC;

    while (clock() < limit) {
      Node *node = root;
      node = select(node);
      node = expand(node);
      int winner = simulate(node);
      backpropagate(node, winner);
    }

    Node *bestChild = nullptr;
    int bestVisit = -1;
    for (auto child : root->children) {
      if (child->visits > bestVisit) {
        bestVisit = child->visits;
        bestChild = child;
      }
    }
    if (bestChild == nullptr) {
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

Move checkDirectWin(HexState &state, int player) {
  for (int i = 1; i <= SIZE; i++) {
    for (int j = 1; j <= SIZE; j++) {
      if (state.board[i][j] == 0) {
        HexState test = state;
        if (test.placeAndUpdate(i, j, player)) {
          if (test.checkWin(player)) {
            return {i, j};
          }
        }
      }
    }
  }
  return {-1, -1};
}

Move getBlockingMove(HexState &state, int player) {
  int opp = -player;
  for (int i = 1; i <= SIZE; i++) {
    for (int j = 1; j <= SIZE; j++) {
      if (state.board[i][j] == 0) {
        HexState test = state;
        if (test.placeAndUpdate(i, j, opp)) {
          if (test.checkWin(opp)) {
            return {i, j};
          }
        }
      }
    }
  }
  return {-1, -1};
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  HexState hex;

#ifndef _BOTZONE_ONLINE
  freopen("in.txt", "r", stdin);
#endif

  int n;
  cin >> n;
  if (!hex.loadFromInput(n,win)) {
    return 0;
  }
  int current_player=1;
  
  int pieces = countPieces(hex);

  Move winMove = checkDirectWin(hex, current_player);
  if (winMove.x >0) {
    cout << winMove.x-1 << " " << winMove.y-1 << endl;
    return 0;
  }

  Move blocking = getBlockingMove(hex, current_player);
  if (blocking.x >0) {
    cout << blocking.x-1 << " " << blocking.y-1 << endl;
    return 0;
  }

  /*if (pieces <= 2) {
      start_first starter(&hex);
      Move best = starter.getBestMove(current_player);
      cout << best.x-1 << " " << best.y-1 << endl;
      return 0;
  }*/

  if (pieces >= 80) {
    MCTS mcts(&hex, current_player);
    Move best = mcts.search(0.9);
    cout << best.x-1 << " " << best.y-1 << endl;
    cout<<check_count_of_node;
    return 0;
  }

  MCTS mcts(&hex, current_player);
  Move best_move = mcts.search(0.85);
  cout << best_move.x-1 << " " << best_move.y-1 << endl;
  cout<<check_count_of_node;
  return 0;
}
