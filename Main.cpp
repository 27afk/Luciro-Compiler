#include <iostream>
#include <vector>
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "SAnalyzer/SAnalyzer.h"
#include "IRgen/IRgen.h"

int main() {
    // 1. Your source code as a raw C-string for your Lexer
    const char* source = R"(
struct Point {
    int x;
    int y;
};

struct Rect {
    Point topLeft;
    Point botRight;
    int ID;
};

void main() {
    Rect campus[5];
    
    campus[0].ID = 101;
    campus[1].botRight.y = 50;
    if((campus[0].ID != 101) || (campus[1].botRight.y == 50)){
    return 0;
}
}
)";

    std::cout << "--- [Luciro Compiler Pipeline] ---\n";

    // 2. Initialize Lexer (Corrected to use const char*)
    Lexer lexer(source);

    // 3. Initialize Parser
    // Your Parser constructor takes Lexer& and internally calls getToken()
    Parser parser(lexer);
    ProgramNode* ast = (ProgramNode*)parser.ParseProgram();
    std::cout << "[Step 1] Parsing Complete.\n";

    // 4. Semantic Analysis
    // SAnalyzer takes a bool for 'freedom' (BaJavMode)
    // We can pull the mode directly from your lexer!
    SAnalyzer analyzer(lexer.firstToken);
    ast->accept(&analyzer);
    std::cout << "[Step 2] Semantic Analysis Complete.\n";

    // 5. IR Generation
    // We pass the struct registry harvested by the analyzer
    IRgen generator(analyzer.getStructRegistry());
    ast->accept(&generator);
    std::cout << "[Step 3] IR Generation Complete.\n";

    // 6. The Catalogue Dump
    generator.Dump();

    return 0;
}
