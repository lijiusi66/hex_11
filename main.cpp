#include <iostream>
#include <vector>
#include <algorithm>
#include "hex_state.h"
#include "MCTS.h"

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    HexState hex; 
    
#ifndef _BOTZONE_ONLINE
    freopen("in.txt", "r", stdin);
#endif
    
    int n;
    cin >> n;
    
    if (!hex.loadFromInput(n)) {
        return 0;
    }
    
    int current_player = (n % 2 == 1) ? 1 : -1;
    
    MCTS mcts(&hex,current_player);
    
    int time_limit_ms;
    if (n <= 5) {
        time_limit_ms = 2000;
    } else if (n <= 15) {
        time_limit_ms = 3000;
    } else {
        time_limit_ms = 4000;
    }
    
    Move best_move = mcts.search(time_limit_ms);
    
    cout << best_move.x << " " << best_move.y << endl;
    
    return 0;
}
