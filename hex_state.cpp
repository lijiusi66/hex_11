#include"hex_state.h"
HexState::HexState(const HexState& other)
{
    *this = other;
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