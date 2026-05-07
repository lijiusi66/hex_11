// mcts.cpp

#include "MCTS.h"

MCTS::Node::Node(const HexState& s)
{
    state = s;

    parent = nullptr;

    move = {-1,-1};

    player = 0;

    visits = 0;

    wins = 0.0;
}

MCTS::Node::Node(Node* p,
                 Move m,
                 int pl)
{
    parent = p;

    move = m;

    player = pl;

    visits = 0;

    wins = 0.0;

    state = p->state;

    state.placeAndUpdate(
        m.x,
        m.y,
        player
    );
}

MCTS::Node::~Node()
{
    for (auto c : children)
    {
        delete c;
    }
}

MCTS::MCTS(HexState* state,
           int player)
{
    rootPlayer = player;

    root = new Node(*state);

    root->untriedMoves =
        generateMoves(root->state);

    srand(time(0));
}

MCTS::~MCTS()
{
    delete root;
}

// uct
double MCTS::uct(Node* parent,
                 Node* child)
{
    if (child->visits == 0)
    {
        return 1e18;
    }

    return
        (child->wins / child->visits)
        +
        1.414 *
        sqrt(log(parent->visits)
        / child->visits);
}

// select
MCTS::Node* MCTS::select(Node* node)
{
    while (
        node->untriedMoves.empty()
        &&
        !node->children.empty()
    )
    {
        double bestValue = -1e18;

        Node* bestChild = nullptr;

        for (auto child : node->children)
        {
            double val = uct(node, child);

            if (val > bestValue)
            {
                bestValue = val;

                bestChild = child;
            }
        }

        node = bestChild;
    }

    return node;
}

// expand
MCTS::Node* MCTS::expand(Node* node)
{
    if (node->untriedMoves.empty())
    {
        return node;
    }

    int idx =
        rand() %
        node->untriedMoves.size();

    Move mv =
        node->untriedMoves[idx];

    node->untriedMoves.erase(
        node->untriedMoves.begin() + idx
    );

    int nextPlayer = -node->player;

    if (node == root)
    {
        nextPlayer = rootPlayer;
    }

    Node* child = new Node(
        node,
        mv,
        nextPlayer
    );

    child->untriedMoves =
        generateMoves(child->state);

    node->children.push_back(child);

    return child;
}

// simulate
int MCTS::simulate(Node* node)
{
    HexState simState = node->state;

    vector<Move> moves =
        generateMoves(simState);

    random_shuffle(
        moves.begin(),
        moves.end()
    );

    int currentPlayer = -node->player;

    if (node == root)
    {
        currentPlayer = rootPlayer;
    }

    int ptr = 0;

    while (true)
    {
        if (simState.checkWin(1))
        {
            return 1;
        }

        if (simState.checkWin(-1))
        {
            return -1;
        }

        if (ptr >= (int)moves.size())
        {
            break;
        }

        Move mv = moves[ptr++];

        if (simState.board[mv.x][mv.y] != 0)
        {
            continue;
        }

        simState.placeAndUpdate(
            mv.x,
            mv.y,
            currentPlayer
        );

        currentPlayer = -currentPlayer;
    }

    if (simState.checkWin(1))
    {
        return 1;
    }

    return -1;
}

// backpropagate
void MCTS::backpropagate(Node* node,
                         int winner)
{
    while (node != nullptr)
    {
        node->visits++;
        if (node->player == winner)
        {
            node->wins += 1.0;
        }
        node = node->parent;
    }
}

// generate moves
vector<Move> MCTS::generateMoves(HexState& state)
{
    vector<Move> moves;

    bool emptyBoard = true;

    for (int i = 1; i <= SIZE; i++)
    {
        for (int j = 1; j <= SIZE; j++)
        {
            if (state.board[i][j] != 0)
            {
                emptyBoard = false;
                break;
            }
        }
    }

    if (emptyBoard)
    {
        moves.push_back(
            {SIZE/2+1, SIZE/2+1}
        );

        return moves;
    }

    near helper(&state);

    for (int i = 1; i <= SIZE; i++)
    {
        for (int j = 1; j <= SIZE; j++)
        {
            if (state.board[i][j] != 0)
            {
                continue;
            }

            if (helper.near_any(i,j)||helper.double_bridge(i,j))
            {
                moves.push_back({i,j});
            }
        }
    }

    return moves;
}

// search
Move MCTS::search(int time_limit_sec)
{
    clock_t start_time = clock();
    clock_t limit =
        start_time +
        time_limit_sec * CLOCKS_PER_SEC;
    /*for (int iter = 0;
         iter < iterations;
         iter++)
    {
        Node* node = root;

        node = select(node);

        node = expand(node);

        int winner =
            simulate(node);

        backpropagate(node,
                      winner);
    }

    Node* bestChild = nullptr;

    int bestVisit = -1;

    for (auto child : root->children)
    {
        if (child->visits > bestVisit)
        {
            bestVisit = child->visits;

            bestChild = child;
        }
    }

    return bestChild->move;*/
    while (clock() < limit)
    {
        Node* node = root;

        node = select(node);

        node = expand(node);

        int winner = simulate(node);

        backpropagate(node, winner);
    }

    Node* bestChild = nullptr;
    int bestVisit = -1;

    for (auto child : root->children)
    {
        if (child->visits > bestVisit)
        {
            bestVisit = child->visits;
            bestChild = child;
        }
    }

    return bestChild->move;
}