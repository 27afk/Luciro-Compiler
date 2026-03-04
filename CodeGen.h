// Change reg tracker to single boolean or int

#pragma once
#include <vector>
#include "SymbolTable.h"
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <array>

/*
enum class IROp {
    // Math
    ADD, SUB, MUL, DIV, MOD,

    // Bitwise (The "Hacker" Ops)
    AND, OR, XOR, SHL, SHR, // XOR, SHL, SHR will be hard coded

    // Comparison
    LT, GT, LE, GE, EQ, NEQ, // <, >, <=, <=, ==, !=

    // Memory (The "Wonky" Core)
    LOAD, STORE, GET_ADDR, ASSIGN, LOAD_CONST, ALLOC,

    // Unary Operators
    NOT, NEG,

    // Flow
    LABEL, JUMP, IF_GOTO, IF_FALSE_GOTO,

    // Functions
    PARAM, CALL, RET,

    // Optimization
    PHI, NOP,

    // promotion/demotion
    ITOF, FTOI
};
*/


// general cpu registers 
enum CpuReg {
    RAX,
    RBX,
    RCX,
    RDX,
    RSI,
    RDI,
    RSP,
    RBP,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    /*
    // 32 bit
    EAX, // RAX
    EBX, // RBX
    ECX, // RCX
    EDX, // RDX
    ESI, // RSI
    EDI, // RDI
    EBP, // RBP
    ESP, // RSP
	R8D,
	R9D,
	R10D,
	R11D,
	R12D,
	R13D,
	R14D,
	R15D*/
};

class CodeGen {
    // stack
    std::string FName;
    std::ofstream asmFile;
	std::vector <std::pair<std::string, CpuReg>>SpillCatalogue; // address and previous register of spilled variable
    int stackOffset; // total stack used
    int currentOffset = 0; // current location in stack
    int totalCapacity;


    // heap
    int HeapOffset;
    int currentHoffset;
    std::vector <int> heap_sizes; // total space
    std::vector <std::pair<int, std::string>> heaps; // size, address in heap
    std::array <int, 16> RegTracker;     // register tracking


    // helper functions
	void ClearReg(CpuReg reg, int size); // clear a register for use
	void ASMwrite(std::string command, bool NLine); // the code u wanna write, asm file name, next line or not
    int HeapCome(int nextRequestSize);
    bool isStack(int size, bool isArray, int ArraySize); // is it an array -1 if its dynamic for the future
    std::string RegToSt(CpuReg reg);

    

    // codegen functions
    void RunTime(); // runtime load
    void StoreGen(); // store variable
    void MallocGen(int size, std::string name);
    void LoadGen(); // load variable value
    

    // Graph Coloring functions & Variables
    std::unordered_map<std::string, double> spillPriorities;

    CpuReg SpillOver(); // sacrifice rax by default :(
    int GetFreeReg();

public:
    // open up file for writing
    CodeGen(std::string fileName);
};