// pseudocode might be a bit yappy cuz vs 2022 AI generates lots of comments and I dont want to lose the comments cuz maybe they work?
#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include "IRgen.h"    // Needed to read Quad structures
#include "CodeGen.h"  // Needed to reference your CpuReg enum

struct GraphNode {
	int id; 
	std::unordered_set<int> neighbors; 
	bool isSpilled = false;
	double spillPriority = 0.0;
};

class RegAllocator {
public  :
	// big boy functions
	RegAllocator(const std::vector<Quad>& quads, StringPool& spool); // builds basic interference graph from quads
	std::unordered_map<int, double> spillPriorities; // for each variable, its spill priority
private :
	// helper functions
	void ClearIntGraph() { interferenceGraph.clear(); }
	void ClearStack() { while (!simplifyStack.empty()) simplifyStack.pop(); }
	void buildInterferenceGraph();
	void RegAllocator::getDefUse(const Quad& q, std::vector<int>& defs, std::vector<int>& uses);
	void calculateSpillPriorities();
	void simplify();

	// helper helper functions
	void addDef(int var, std::vector<int>& defs);
	void addUse(int var, std::vector<int>& uses);
	void selectRegisters();
	void addInterferenceEdge(int u, int v);

	std::vector<Quad> quads; // the IR quads to analyze for interference
	std::unordered_map<int, GraphNode> interferenceGraph; // key: variable ID, value: graph node with neighbors
	std::stack<int> simplifyStack; // for the simplification phase
	std::vector <int> spilledVariables; // to track which variables got spilled
	std::vector <int> finalSpilledVariables; // to track which spilled variables are actually used in the final codegen phase so we dont waste time on spilled variables that are never used again
	StringPool& pool;
	std::unordered_map<int, CpuReg> getAllocation() const { return allocation; } // final output of register allocation, mapping variable ID to register
	std::unordered_map<int, CpuReg> allocation; // final mapping of variable ID to register    
};