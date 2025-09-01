#include <random>



namespace Zobrist{
	uint64_t zobristTable[12][8][8]; // 6 piece types * 2 colors
	uint64_t zobristCastling[4];   // KQkq
	uint64_t zobristEnPassant[8];  // files a–h
	uint64_t zobristSideToMove;    // black to move

	void initializeZobrist(){
		std::mt19937_64 rng(0xDEADBEEF); // Fixed seed for reproducibility
    	std::uniform_int_distribution<uint64_t> dist;

    	for(int i = 0; i < 12; i++){
    		for(int j = 0; j < 8; j++){
    			for(int k = 0; k < 8; k++){
    				zobristTable[i][j][k] = dist(rng);
    			}
    		}
    	}

    	for(int i = 0; i < 4; i++){
    		zobristCastling[i] = dist(rng);
    		zobristEnPassant[i] = dist(rng);
    		zobristEnPassant[i+4] = dist(rng);
    	}

    		zobristSideToMove = dist(rng);
	}
}