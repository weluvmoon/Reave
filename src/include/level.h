#include "data.h"
#include "entities.h"
#include <raymath.h>

// Components
struct WaveFunction {
    std::vector<int> possibleTiles; // IDs of allowed tiles
    bool collapsed = false;
};

struct CellPosition {
    int x, y;
};

struct WFCSystem {
  public:
    void update(Registry &registry) {
        // 1. Find lowest entropy cell
        // 2. Select tile
        // 3. Propagate to neighbors
        // 4. Update entities
    }
};

struct LevelManager {
    bool Save(const std::string &filename);
    bool Load(const std::string &filename);
    void Clear();
};

extern LevelManager lm;
