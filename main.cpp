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
    int threshold = 0.95 * (double)CLOCKS_PER_SEC; 
    Move best_move = mcts.search(0.95);
    cout << best_move.x << " " << best_move.y << endl;
    
    return 0;
}
