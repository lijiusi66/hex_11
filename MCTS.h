#ifndef MCTS_H
#define MCTS_H

#include "hex_state.h"
#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>

using namespace std;

class MCTS {
private:

    struct Node {

        HexState state;

        Node* parent;

        vector<Node*> children;

        Move move;

        int player; // 当前节点是谁下的这一步

        int visits;

        double wins;

        vector<Move> untriedMoves;

        Node(const HexState& s);
        Node(
             Node* p,
             Move m,
             int pl);
            
        ~Node();
    };

private:

    Node* root;

    near* nr;

    int rootPlayer;

    int dx[6] = {-1,-1,0,0,1,1};
    int dy[6] = {0,1,-1,1,-1,0};

public:

    MCTS(HexState* state, int player);

    ~MCTS();

    Move search(double iterations);

private:

    Node* select(Node* node);

    Node* expand(Node* node);

    int simulate(Node* node);

    void backpropagate(Node* node, int winner);

    double uct(Node* parent, Node* child);

    vector<Move> generateMoves(HexState& state);

};

#endif