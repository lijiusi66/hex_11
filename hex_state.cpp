#include"hex_state.h"
#include<iostream>
// 六个方向的偏移量
const int DX[6] = {-1, -1, 0, 0, 1, 1};
const int DY[6] = {0, 1, -1, 1, -1, 0};

void HexState::initUnionFind(int player) {
  int idx = (player == 1) ? 0 : 1;
  uf[idx].init();
  uf_initialized[idx] = true;

  // 遍历所有棋盘位置，连接相邻的同色棋子
  for (int x = 1; x <= SIZE; x++) {
    for (int y = 1; y <= SIZE; y++) {
      if (board[x][y] != player)
        continue;

      int id = pos2id(x, y);

      // 检查是否连接到边界（虚拟节点）
      if (player == 1) {
        // 玩家1（先手/红方）：连接上下边界 (y=1 是上，y=SIZE 是下)
        if (y == 1)
          uf[idx].unite(id, VIRTUAL_TOP);
        if (y == SIZE)
          uf[idx].unite(id, VIRTUAL_BOTTOM);
      } else {
        // 玩家-1（后手/蓝方）：连接左右边界 (x=1 是左，x=SIZE 是右)
        if (x == 1)
          uf[idx].unite(id, VIRTUAL_LEFT);
        if (x == SIZE)
          uf[idx].unite(id, VIRTUAL_RIGHT);
      }

      // 连接六个方向的邻居
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

bool HexState::checkWin(int player) {
  int idx = (player == 1) ? 0 : 1;

  // 延迟初始化
  if (!uf_initialized[idx]) {
    initUnionFind(player);
  }

  // 检查两个虚拟节点是否连通
  if (player == 1) {
    // 玩家1（先手/红方）：检查上下是否连通
    return uf[idx].connected(VIRTUAL_TOP, VIRTUAL_BOTTOM);
  } else {
    // 玩家-1（后手/蓝方）：检查左右是否连通
    return uf[idx].connected(VIRTUAL_LEFT, VIRTUAL_RIGHT);
  }
}

bool HexState::placeAndUpdate(int x, int y, int player) {
  if (!in_board(x, y) || board[x][y] != 0)
    return false;

  board[x][y] = player;
  int idx = (player == 1) ? 0 : 1;
  int id = pos2id(x, y);

  // 如果并查集未初始化，需要全量初始化
  if (!uf_initialized[idx]) {
    initUnionFind(player);
    return true;
  }

  // 增量更新：连接边界
  if (player == 1) {
    // 玩家1（先手/红方）：连接上下边界
    if (y == 1)
      uf[idx].unite(id, VIRTUAL_TOP);
    if (y == SIZE)
      uf[idx].unite(id, VIRTUAL_BOTTOM);
  } else {
    // 玩家-1（后手/蓝方）：连接左右边界
    if (x == 1)
      uf[idx].unite(id, VIRTUAL_LEFT);
    if (x == SIZE)
      uf[idx].unite(id, VIRTUAL_RIGHT);
  }

  // 增量更新：连接邻居
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
HexState::HexState(const HexState& other)
{
    *this = other;
}
bool HexState::loadFromInput(int n){
    int x,y;
    for (int i = 0; i < n - 1; i++) {
		std::cin >> x >> y; if (x != -1) board[x][y] = -1;	//对方
		std::cin >> x >> y; if (x != -1) board[x][y] = 1;	//我方
	}
	std::cin >> x >> y;
	if (x != -1) board[x][y] = -1;	//对方
	else { std::cout << 1 << ' ' << 2 << std::endl;	return false; }  //强制第一手下在C2
    return true;
}
HexState& HexState::operator=(const HexState& other)
{
    if (this == &other)
        return *this;

    // copy board
    for (int i = 0; i < SIZE + 2; i++)
    {
        for (int j = 0; j < SIZE + 2; j++)
        {
            board[i][j] = other.board[i][j];
        }
    }

    // copy union find
    uf[0] = other.uf[0];
    uf[1] = other.uf[1];

    uf_initialized[0] = other.uf_initialized[0];
    uf_initialized[1] = other.uf_initialized[1];

    return *this;
}