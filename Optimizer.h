#pragma once
#include "IRgen/IRgen.h"
#include <map>


class LuciroOptimizer {
    int tempCount;
    StringPool& Spool;
    std::map <int, int> substitutions;
public:
    // Pass the current count from IRGen so we don't collide
    LuciroOptimizer(StringPool& spool, int startTempCount) 
        : Spool(spool), tempCount(startTempCount) {}

    int nextTemp() {
        return Spool.getOrCreate("t" + std::to_string(tempCount++));
    }
    bool isNumber(std::string val);
    std::vector<Quad> optimize(const std::vector<Quad>& oldInstructions, StringPool& spool);
};