#include "RegAllocator.h"
#include <cctype>
#include <vector>

RegAllocator::RegAllocator(const std::vector<Quad>& quads, StringPool& spool) : pool(spool), quads(quads) {
    buildInterferenceGraph();
    calculateSpillPriorities();
    simplify();
	selectRegisters();
} // builds basic interference graph from quads

void RegAllocator::addUse(int var, std::vector<int>& uses) {
    
    if (var == -1) return; // return if error

    // get the thing from spoooollllio
    std::string name = std::string(pool.getName(var));
    if (name.empty()) return;

	if (std::isdigit(name[0]) || name[0] == '-' || name[0] == 'L') return; // Skip constants and labels

	uses.push_back(var);
}

void RegAllocator::addDef(int var, std::vector<int>& defs) {

    if (var == -1) return; // return if error

    // get the thing from spoooollllio
    std::string name = std::string(pool.getName(var));
    if (name.empty()) return;

    if (std::isdigit(name[0]) || name[0] == '-' || name[0] == 'L') return; // Skip constants and labels

    defs.push_back(var);
}

void RegAllocator::getDefUse(const Quad& q, std::vector<int>& defs, std::vector<int>& uses) {
    switch (q.op) {
        // --- Standard Math / Comparisons / Casts ---
    case IROp::ADD:   case IROp::SUB:   case IROp::MUL:   case IROp::DIV:   case IROp::MOD:
    case IROp::F_ADD: case IROp::F_SUB: case IROp::F_MUL: case IROp::F_DIV:
    case IROp::EQ:    case IROp::NEQ:   case IROp::LT:    case IROp::GT:    case IROp::LE: case IROp::GE:
        addDef(q.res, defs);
        addUse(q.arg1, uses);
        addUse(q.arg2, uses);
        break;

    case IROp::NOT:   case IROp::NEG:   case IROp::NEG_F:
    case IROp::ITOF:  case IROp::FTOI:
    case IROp::LOAD:
        addDef(q.res, defs);
        addUse(q.arg1, uses);
        break;

    case IROp::LOAD_CONST:
    case IROp::CALL:
        addDef(q.res, defs);
        break;

    case IROp::ASSIGN:
        addDef(q.res, defs);
        addUse(q.arg2, uses); // Value is strictly stored in q.arg2 per your emitter configuration
        break;

        // --- Store Memory Modification Exception ---
    case IROp::STORE:
        addUse(q.res, uses);  // Base address/offset pointer register
        addUse(q.arg1, uses); // Target contents register
        break;

        // --- Function Linkage & Program Layout Rules ---
    case IROp::PARAM:
        addUse(q.res, uses);  // Variable or register being prepared as an argument
        break;

    case IROp::IF_FALSE_GOTO:
        addUse(q.arg1, uses); // Target boolean condition tracking register
        break;

    case IROp::RET:
        if (q.arg1 != -1) {
            addUse(q.arg1, uses); // Return value tracking register
        }
        break;

    default:
        // ALLOC, LABEL, JUMP, NOP do not manipulate variables via CPU register slots
        break;
    }
}

void RegAllocator::selectRegisters() {

    const CpuReg hardwareRegisters[11] = {
        CpuReg::R10, CpuReg::R11, CpuReg::R12, CpuReg::R13,
        CpuReg::R14, CpuReg::RBX, CpuReg::RDI, CpuReg::RSI,
        CpuReg::RCX, CpuReg::R8,  CpuReg::R9
    };

	const int K = 11; // no div and no other 3 regs so only 11 registers left for allocation



    // Pop nodes off your simplifyStack until it is completely empty
    while (!simplifyStack.empty()) {
        int nodeID = simplifyStack.top();
        simplifyStack.pop();

        std::unordered_set<CpuReg> forbiddenRegisters;

        for (int neighborID : interferenceGraph[nodeID].neighbors) {
            if (allocation.find(neighborID) != allocation.end()) {
                forbiddenRegisters.insert(allocation[neighborID]);
            }
        }

        CpuReg chosenRegister;
        bool foundColor = false;

        for (int i = 0; i < K; ++i) {
            CpuReg candidate = hardwareRegisters[i];
            if (forbiddenRegisters.find(candidate) == forbiddenRegisters.end()) {
                chosenRegister = candidate;
                foundColor = true;
                break;
            }
        }

        if (foundColor) { // no spill
            allocation[nodeID] = chosenRegister;
        }
        else { // yes spill
            spilledVariables.push_back(nodeID);
            interferenceGraph[nodeID].isSpilled = true;
        }
    }
}

// create the graph to color
void RegAllocator::buildInterferenceGraph() {
    std::unordered_set<int> liveSet;

    // trace from backwards to determine true lifetimes
    for (int i = quads.size() - 1; i >= 0; --i) {
        const Quad& q = quads[i];

        std::vector<int> defs;
        std::vector<int> uses;
        getDefUse(q, defs, uses);

        for (int def : defs) {
            liveSet.erase(def);
        }
        // interferences
        for (int def : defs) {
            if (!interferenceGraph.count(def)) {
                interferenceGraph[def] = GraphNode{ def };
            }

            for (int liveVar : liveSet) {
                if (def != liveVar) {
                    addInterferenceEdge(def, liveVar);
                }
            }
        }

        for (int use : uses) {
            // Ensure the node exists in the graph
            if (!interferenceGraph.count(use)) {
                interferenceGraph[use] = GraphNode{ use };
            }
            liveSet.insert(use);
        }
    }
}

void RegAllocator::addInterferenceEdge(int u, int v) {
    interferenceGraph[u].neighbors.insert(v);
    interferenceGraph[v].neighbors.insert(u);
}

void RegAllocator::calculateSpillPriorities() {
    std::unordered_map<int, int> degreeMap; // variable ID to degree mapping for spill priority calculation
    for (const auto& q: quads) {
		std::vector<int> defs;
        std::vector<int> uses;
		getDefUse(q, defs, uses);
        for (int def : defs) {
            degreeMap[def]++; 
        }
        for (int use : uses) {
            degreeMap[use]++;
		}
    }

    for ( const auto & pair : interferenceGraph) {
        // The number of neighbors is the actual mathematical degree of the node
        int numNeighbors = pair.second.neighbors.size();
		const GraphNode& node = pair.second;

        if (numNeighbors == 0) {
            spillPriorities[pair.first] = 1e9; // Safe infinite priority, costs nothing to color
            continue;
        }

        int count = degreeMap[pair.first];

        // prioirity = loop nest weight * use/derf count)/number of neighbors        
        spillPriorities[pair.first] = static_cast<double>(count) / numNeighbors;
    }
}

void RegAllocator::simplify() {

    std::unordered_map<int, std::unordered_set<int>> workingGraph; 
    std::unordered_set<int> remainingNodes;

    for (const auto& pair : interferenceGraph) {
		workingGraph[pair.first] = pair.second.neighbors;
		remainingNodes.insert(pair.first);
    }

    const int K = 11; // General purpose physical register limit but only 13 since 3 of them r used to track stack and - 2 for div

    while (!remainingNodes.empty()) {
        bool foundSimplifiable = false;
        int targetNode = -1;

        for (int node : remainingNodes) {
            if (workingGraph[node].size() < K) {
                targetNode = node;
                foundSimplifiable = true;
                break; 
            }
        }

        if (!foundSimplifiable) {
            double lowestPriority = 1e18;
            int victim = -1;

            for (int node : remainingNodes) {
                if (spillPriorities[node] < lowestPriority) {
                    lowestPriority = spillPriorities[node];
                    victim = node;
                }
            }

            targetNode = victim;
            spilledVariables.push_back(targetNode);
        }

        remainingNodes.erase(targetNode);

        if (foundSimplifiable) {
            simplifyStack.push(targetNode);
        }
        else {
            // It's a spill candidate! It won't get a register color, 
            // so we don't push it to the simplifyStack.
        }

        for (int neighbor : workingGraph[targetNode]) {
            workingGraph[neighbor].erase(targetNode);
        }
        workingGraph[targetNode].clear();
    }
}