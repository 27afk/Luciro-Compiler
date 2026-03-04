#pragma once
#include <unordered_map>
#include <string>
#include <vector>

struct StackSlot {
    std::string name;
    int offset;
    int size;
    bool isArray;
    int Arraysize; // -1 if dynamic
};

struct Symbol {
    std::string name;
    int id;                 // Unique ID (e.g., %1, %2)
    int size;               // 4 for int, 8 for long/pointers

    // Register Allocation Data
    bool isSpilled = false;
    int stackOffset = -1;
    double priority = 0.0;
    int loopDepth = 0;

    // For Graph Coloring
    std::vector<int> neighbors; // IDs of interfering variables
};

class SymbolTable {
private:
    std::unordered_map<std::string, int> nameToId;
    std::vector<Symbol> symbols; // Indexed by ID
    int nextId = 0;

public:
    // Create or find a variable
    int getOrCreateSymbol(std::string name, int size) {
        if (nameToId.find(name) == nameToId.end()) {
            Symbol s;
            s.name = name;
            s.id = nextId;
            s.size = size;
            symbols.push_back(s);
            nameToId[name] = nextId;
            return nextId++;
        }
        return nameToId[name];
    }

    // Direct access to metadata via ID
    Symbol& getSymbol(int id) { return symbols[id]; }

    size_t count() const { return symbols.size(); }
};