#include "hex_state.h"
#include <cstring>
#include <iostream>

using namespace std;

HexState::HexState() { init(); }

void HexState::init() { memset(board, 0, sizeof(board)); }

bool HexState::in_board(int x, int y) const {
  return x >= 1 && x <= SIZE && y >= 1 && y <= SIZE;
}

bool HexState::place(int x, int y, int player) {
  if (!in_board(x, y))
    return false;
  if (board[x][y] != 0)
    return false;
  board[x][y] = player;
  return true;
}

bool HexState::loadFromInput(int n) {
  int x, y;
  // 恢复目前的棋盘信息
  for (int i = 0; i < n - 1; i++) {
    cin >> x >> y;
    if (x != -1)
      board[x + 1][y + 1] = -1; // 对方
    cin >> x >> y;
    if (x != -1)
      board[x + 1][y + 1] = 1; // 我方
  }
  cin >> x >> y;
  if (x != -1)
    board[x][y] = -1; // 对方
  else {
    cout << 1 << ' ' << 2 << endl;
    return false;
  } // 强制第一手下在C2
  /*
  for (int i = 0; i < 13; i++) {
    board[0][i] = 2;        // 上边
    board[12][i] = 2;       // 下边
    board[i][0] = 2;       // 左边
    board[i][12] = 2;      // 右边
}*/
}