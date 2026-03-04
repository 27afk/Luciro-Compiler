#include "CodeGen.h"
#include "SymbolTable.h"
#include <fstream>
#include <string>
#include <iostream>

CodeGen::CodeGen(std::string fileName) : FName(fileName), asmFile(fileName) {
    totalCapacity = 1000000;
    heap_sizes.push_back(1000000);
    HeapOffset = 0;
    stackOffset = 0;

    if (!asmFile.is_open()) {
        std::cerr << "Fatal Error: Could not open ASM file " << fileName << std::endl;
        return;
    }

    // Initialize the Register Tracker
    // pair.first = Status (0: Free, 1: Busy, 2: Locked/Sacred)
    // pair.second = The Enum identity          
    for (int i = 0; i < 16; i++) {
        CpuReg current = static_cast<CpuReg>(i);

        // Mark Sacred Registers so the allocator never picks them
        if (current == CpuReg::RSP || current == CpuReg::RBP || current == CpuReg::R15) {
            RegTracker[i] = current ;
        }
        else {
            RegTracker[i] = current ;
        }
    }
}



void CodeGen::ASMwrite(std::string command, bool NLine) {
    asmFile << command;
    if (NLine) {
        asmFile << std::endl;
    }
}

void CodeGen::ClearReg(CpuReg reg, int size) {
	RegTracker[(int)reg] = 0; // Mark the register as free
	stackOffset += size; 
}



CpuReg CodeGen::SpillOver() {
    return CpuReg::RAX;
}

int CodeGen::GetFreeReg() {
    for (int i = 0; i < 16; i++) {
        if (RegTracker[i] == 0) {
            return i;
        }
    }
    return static_cast<int>(SpillOver());
}

std::string CodeGen::RegToSt(CpuReg reg) {
    switch (reg) {
    // 64 bit registers
    case CpuReg::RAX: return "rax";
    case CpuReg::RBX: return "rbx";
    case CpuReg::RCX: return "rcx";
    case CpuReg::RDX: return "rdx";
    case CpuReg::RSI: return "rsi";
    case CpuReg::RDI: return "rdi";
    case CpuReg::RBP: return "rbp"; // DO NOT TOUCH! RBP
    case CpuReg::RSP: return "rsp"; // DO NOT TOUCH! RSP
    case CpuReg::R8:  return "r8";
    case CpuReg::R9:  return "r9";
    case CpuReg::R10: return "r10";
    case CpuReg::R11: return "r11";
    case CpuReg::R12: return "r12";
    case CpuReg::R13: return "r13";
    case CpuReg::R14: return "r14";
    case CpuReg::R15: return "r15"; // LOCKED DONT TOUCH HEAP WILL EXPLODE NONONONO!! :(((( r15 NONONO r15 DONT TOUCH!!!
    // 32 bit registers
        /*
	case CpuReg::EAX: return "eax";
	case CpuReg::EBX: return "ebx";
	case CpuReg::ECX: return "ecx";
	case CpuReg::EDX: return "edx";
	case CpuReg::ESI: return "esi";
	case CpuReg::EDI: return "edi";
	case CpuReg::EBP: return "ebp"; // DO NOT TOUCH! RBP
	case CpuReg::ESP: return "esp"; // DO NOT TOUCH! RSP
	case CpuReg::R8D: return "r8d";
	case CpuReg::R9D: return "r9d";
	case CpuReg::R10D: return "r10d";
	case CpuReg::R11D: return "r11d";
	case CpuReg::R12D: return "r12d";
	case CpuReg::R13D: return "r13d";
	case CpuReg::R14D: return "r14d";
	case CpuReg::R15D: return "r15d";*/ // LOCKED DONT TOUCH HEAP WILL EXPLODE NONONONO!! :(((( r15 NONONO r15 DONT TOUCH!!!
    default:          return "UNKNOWN_REG";
    }
}

void CodeGen::RunTime() {
    ASMwrite("section .text", true);
    ASMwrite("  extern malloc", true);
    ASMwrite("  extern realloc", true);
    ASMwrite("  extern free", true);

    // Initial setup: Get the first 1MB from the OS
    totalCapacity = 1000000;
    ASMwrite("  mov rdi, " + std::to_string(static_cast<int>(totalCapacity)), true);
    ASMwrite("  call malloc", true);
    ASMwrite("  mov r15, rax", true); // r15 is now the anchor for everything
    // for future add STL
}

bool CodeGen::isStack(int size, bool isArray, int ArraySize) {
    if (!isArray) {
        if (((size + stackOffset) > 1000000) || size > 16000) {
            return false;
        }
    }
    else {
        size = ArraySize * size;
        if (((size + stackOffset) > 1000000) || size > 16000) {
            return false;
        }
    }
    return true;
}



int CodeGen::HeapCome(int nextRequestSize) {
    // if not full just leave
    if (HeapOffset + nextRequestSize < totalCapacity) {
        return 0;
    }
    // if full return the amount that it spills over 
    return (HeapOffset + nextRequestSize - totalCapacity);
}

// for objects heap allocation
void CodeGen::MallocGen(int size, std::string name) {
    int spillAmount = HeapCome(size);

    if (spillAmount > 0) {
        int newBlock = 1000000;
        totalCapacity += newBlock;
        heap_sizes.push_back(newBlock);

        // Tell the OS to expand the block pointed to by r15
        ASMwrite("mov rdi, r15", true);                            // Pointer to current heap
        ASMwrite("mov rsi, " + std::to_string(totalCapacity), true); // New TOTAL size
        ASMwrite("call realloc", true);
        ASMwrite("mov r15, rax", true);                            // Update r15 (it might have moved!)
    }

    // Map the variable to the CURRENT offset
    heaps.push_back({ HeapOffset, name });

    // Move the pointer forward for the next 
    HeapOffset += size;
}

