#include "hex_state.h"
#include <iostream>
#include <cstring>

using namespace std;

HexState::HexState() {
    init();
}

void HexState::init() {
    memset(board, 0, sizeof(board));
}

bool HexState::in_board(int x, int y) const {
    return x >= 0 && x < SIZE && y >= 0 && y < SIZE;
}

bool HexState::place(int x, int y, int player) {
    if (!in_board(x, y)) return false;
    if (board[x][y] != 0) return false;

    board[x][y] = player;
    return true;
}

void HexState::loadFromInput() {
    int x, y, n;
    cin >> n;

    for (int i = 0; i < n - 1; i++) {
        cin >> x >> y;
        if (x != -1) place(x, y, -1);

        cin >> x >> y;
        if (x != -1) place(x, y, 1);
    }

    cin >> x >> y;
    if (x != -1) place(x, y, -1);
    else {
        // 锁定先手开局
        cout << 1 << ' ' << 2 << endl;
        exit(0);
    }
}