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

	cout << new_x << ' ' << new_y << endl;
	return 0;
}
