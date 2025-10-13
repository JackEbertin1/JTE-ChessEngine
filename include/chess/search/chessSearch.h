#include "chess/evaluation/Evaluation.h"
#include <future>
#include <numeric>  // for std::accumulate
#include <memory> 
#include <mutex>
#include <shared_mutex>
#include <algorithm>


using namespace Evaluation;


namespace chessSearch {
	// Global variable declarations
	extern int nodesVisited;
	extern int startingDepth;
	extern int repeatedPositions;

	struct TTEntry {
		float value;
		int depth;
		Move bestMove;
		enum Flag { EXACT, LOWERBOUND, UPPERBOUND } type;
	};

	extern std::unordered_map<uint64_t, TTEntry> transpositionTable;
	extern std::mutex ttMutex;

	// Function declarations
	void sortMoves(MoveList& moves, Board* board);
	void storeTTEntry(uint64_t zobristKey, float value, int depth, float alphaOrig, float beta,  Move bestMove);

	float minimax(Board* board, int depth, bool maximizingPlayer);

	float minimaxAB(Board* board, int depth, bool maximizingPlayer, float alpha, float beta);

	std::pair<std::string, float> searchBestMoveParallel(Board* board, int depth, bool maximizingPlayer);

	std::pair<std::string, float> searchBestMove(Board* board, int depth, bool maximizingPlayer);
}


