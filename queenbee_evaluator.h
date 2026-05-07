#ifndef QUEENBEE_EVALUATOR_H
#define QUEENBEE_EVALUATOR_H

#include "hex_state.h"
#include <queue>

class QueenbeeEvaluator {
public:
    
    QueenbeeEvaluator(HexState* s);
    int evaluate(HexState* s, int player);

private:
    
    void computeDistances(int player, int resA[SIZE+2][SIZE+2], int resB[SIZE+2][SIZE+2]);

  
    int getQueenbeePotential(int player);
    bool isBoundary(int x, int y, int player) const;

    
    inline int playerIndex(int player) {
        return (player == 1) ? 0 : 1;
    }

    HexState* state;
    int size;        

    
    int distA[2][SIZE+2][SIZE+2];
    int distB[2][SIZE+2][SIZE+2];
};

#endif // QUEENBEE_EVALUATOR_H