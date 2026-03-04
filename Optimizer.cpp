/*
To do:
Add implicit casting
Finish optimization

*/
#include "IRgen/IRgen.h"
#include "Optimizer.h"
#include <string>
#include <cctype>
#include <map>
// for mat emit ( IROp command, variable ID, size of variable, init value so -1 means none)
bool LuciroOptimizer::isNumber(std::string val) {
    int start = 0;
    if (val[0] == '-') { // for negatives
        start++;
    }
    for (int i = start; i < val.length(); i++) {
        if (!isdigit(val[i])) {
            return false;
        }
    }
    return true;
}

std::vector<Quad> LuciroOptimizer::optimize(const std::vector<Quad>& oldInstructions, StringPool& spool) {
    std::vector<Quad> newInstructions;

    for (const auto& q : oldInstructions) {
        int a1 = q.arg1;
        int a2 = q.arg2;
        if (substitutions.count(a1)) a1 = substitutions[a1];
        if (substitutions.count(a2)) a2 = substitutions[a2];

        if (q.op == IROp::ASSIGN) {
            substitutions[q.res] = a2;
            continue;
        }
        //ADD, SUB, MUL, DIV, MOD,
        if (q.op == IROp::ADD) {
            std::string val1 = spool.getName(a1);
            std::string val2 = spool.getName(a2);

            if (isNumber(val1) && isNumber(val2)) {
                // DO THE MATH NOW
                int result = std::stoi(val1) + std::stoi(val2);

                // Register the new constant and substitute it
                int newConstId = spool.getOrCreate(std::to_string(result));
                int newReg = nextTemp();

                newInstructions.push_back({ IROp::LOAD_CONST, newReg, newConstId, -1 });
                substitutions[q.res] = newReg;
                continue; 
            }
        }
        if (q.op == IROp::SUB) {
            std::string val1 = spool.getName(a1);
            std::string val2 = spool.getName(a2);
            if (isNumber(val1) && isNumber(val2)) {
                int result = std::stoi(val1) - std::stoi(val2);
                int newConstId = spool.getOrCreate(std::to_string(result));
                int newReg = nextTemp();

                newInstructions.push_back({ IROp::LOAD_CONST, newReg, newConstId, -1 });
                substitutions[q.res] = newReg;
                continue;
            }
        }

        Quad optimizedQ = q;
        optimizedQ.arg1 = a1;
        optimizedQ.arg2 = a2;
        newInstructions.push_back(optimizedQ);
    }
    return newInstructions;
}