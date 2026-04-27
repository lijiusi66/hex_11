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
<<<<<<< HEAD
    return x >= 0 && x < SIZE && y >= 0 && y < SIZE;
=======
    return x >= 1 && x <= SIZE && y >= 1 && y <= SIZE;
>>>>>>> ff6a46c (init)
}

bool HexState::place(int x, int y, int player) {
    if (!in_board(x, y)) return false;
    if (board[x][y] != 0) return false;

    board[x][y] = player;
    return true;
}

<<<<<<< HEAD
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
=======
bool HexState::loadFromInput(int n) {
    int x, y;
	//恢复目前的棋盘信息
	for (int i = 0; i < n - 1; i++) {
		cin >> x >> y; if (x != -1) board[x+1][y+1] = -1;	//对方
		cin >> x >> y; if (x != -1) board[x+1][y+1] = 1;	//我方
	}
	cin >> x >> y;
	if (x != -1) board[x][y] = -1;	//对方
	else { cout << 1 << ' ' << 2 << endl;	return false;}  //强制第一手下在C2
	for(int i=1;i<13;i++)board[0][i]=2;
	for(int i=1;i<13;i++)board[i][12]=2;
	for(int i=1;i<13;i++)board[12][12-i]=2;
	for(int i=0;i<12;i++)board[i][0]=2;
>>>>>>> ff6a46c (init)
}