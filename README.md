# Chess Engine

A high-performance C++ chess engine with complete chess rule implementation, advanced search algorithms, and evaluation functions.

## Project Structure

```
chess/
├── include/                    # Header files
│   └── chess/
│       ├── Board.h            # Main board representation and game logic
│       ├── Move.h             # Move representation and validation
│       ├── Zobrist.h          # Zobrist hashing for position identification
│       ├── pieces/            # Chess piece implementations
│       │   ├── Piece.h        # Abstract base class for all pieces
│       │   ├── King.h         # King piece implementation
│       │   ├── Queen.h        # Queen piece implementation
│       │   ├── Rook.h         # Rook piece implementation
│       │   ├── Bishop.h       # Bishop piece implementation
│       │   ├── Knight.h       # Knight piece implementation
│       │   └── Pawn.h         # Pawn piece implementation
│       ├── search/            # Search algorithms
│       │   ├── chessSearch.h  # Minimax with alpha-beta pruning
│       │   └── Perft.h        # Perft testing for move generation
│       └── evaluation/        # Position evaluation
│           └── Evaluation.h   # Material and positional evaluation
├── src/                       # Source files
│   └── main.cpp              # Main program entry point
├── obj/                       # Object files (generated during build)
├── bin/                       # Executable output (generated during build)
├── lib/                       # Library files (if any)
├── Makefile                  # Traditional make build system
├── CMakeLists.txt            # Modern CMake build system
└── README.md                 # This file
```

## Features

- **Complete Chess Rules**: All standard chess rules including castling, en passant, and promotion
- **Advanced Search**: Minimax with alpha-beta pruning and transposition tables
- **Parallel Search**: Multi-threaded search for root moves
- **Position Evaluation**: Material counting and piece mobility scoring
- **Zobrist Hashing**: Efficient position identification and caching
- **FEN Support**: Standard chess notation input/output
- **Move Validation**: Legal move generation and validation
- **Perft Testing**: Move generation testing and validation

## Building the Project

### Option 1: Using Make (Traditional)

```bash
# Build the project
make

# Build with debug symbols
make debug

# Build with optimization
make release

# Clean build files
make clean

# Show available targets
make help
```

### Option 2: Using CMake (Modern)

```bash
# Create build directory
mkdir build && cd build

# Configure the project
cmake ..

# Build the project
cmake --build .

# Build with specific configuration
cmake --build . --config Release
cmake --build . --config Debug

# Enable additional features
cmake -DENABLE_SANITIZERS=ON -DENABLE_PROFILING=ON ..
```

## Running the Engine

```bash
# Run the compiled engine
./bin/chessEngine

# Or if using CMake build
./build/bin/chessEngine
```

## Usage

The chess engine provides an interactive command-line interface:

- **FEN Input**: Enter FEN strings to start from specific positions
- **Move Input**: Enter moves in standard chess notation (e.g., "e2e4")
- **Commands**:
  - `wm` - Show white's legal moves
  - `bm` - Show black's legal moves
  - `FEN` - Display current position in FEN notation
  - `pieces` - Show all alive pieces
  - `print` - Display the current board
  - `perft` - Run perft testing
  - `undo` - Undo the last move
  - `Quit` - Exit the program

## Development

### Adding New Features

1. **New Pieces**: Add header files to `include/chess/pieces/`
2. **New Search Algorithms**: Add to `include/chess/search/`
3. **New Evaluation Functions**: Add to `include/chess/evaluation/`
4. **Core Logic**: Modify `include/chess/Board.h`

### Code Style

- Use C++20 features
- Follow RAII principles
- Use smart pointers where appropriate
- Maintain const-correctness
- Document public interfaces

### Testing

The engine includes perft testing for move generation validation:

```bash
# Run perft test
perft
# Enter depth (e.g., 4)
```

## Performance

- **Move Generation**: O(1) per piece (fixed maximum moves)
- **Search**: Alpha-beta pruning with move ordering
- **Memory**: Efficient piece representation and caching
- **Parallelism**: Multi-threaded root move search

## Dependencies

- **Compiler**: GCC 10+ or Clang 12+ with C++20 support
- **Standard Library**: C++20 standard library
- **Build System**: Make or CMake 3.16+

## License

[Add your license information here]

## Contributing

[Add contribution guidelines here]
