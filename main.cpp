#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include<vector>
#include<algorithm>
#include <cstring>   // C++推荐写法
#include"hex_state.h"
using namespace std;

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    HexState hex; 
#ifndef _BOTZONE_ONLINE
	freopen("in.txt", "r", stdin);
#endif // !_BOTZONE_ONLINE
<<<<<<< HEAD

	int x, y, n;
	//恢复目前的棋盘信息
	cin >> n;
	for (int i = 0; i < n - 1; i++) {
		cin >> x >> y; if (x != -1) hex.board[x][y] = -1;	//对方
		cin >> x >> y; if (x != -1) hex.board[x][y] = 1;	//我方
	}
	cin >> x >> y;
	if (x != -1) hex.board[x][y] = -1;	//对方
	else { cout << 1 << ' ' << 2 << endl;	return 0; }  //强制第一手下在C2
	
	int new_x,new_y;
    


	// 向平台输出决策结果
=======
	int n;
	cin>>n;
	if(!hex.loadFromInput(n))cout << 1 << ' ' << 2 << endl;
	else{
		if(n<=5){
			
		}else{

		}
	}
	int new_x,new_y;
	// 向平台输出决策结果

>>>>>>> ff6a46c (init)
	cout << new_x << ' ' << new_y << endl;
	return 0;
}
