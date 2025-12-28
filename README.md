Key features
- Nested Structure support
- #BaJav# mode AKA error suppression mode to pull off wonky stuff ( to access BaJav mode just type "#BaJav#" at the front of ur program )
- 64 bit memory model
- simple Scope Management
- Arrays
Architecture
Lexer
- a simple TokenType class to help with simple token lexing
- flow control like while loop and if statements
Parser
- Uses an AST to represent the lexed tokens
- Uses a separate clothesline model to save data of the nodes and to avoid memory leaks
Semantic Analysis
- Type Checking: Ensures compatibility between targets and sources during assignments.
- Size Calculation: Looks up struct blueprints in a structRegistry to determine exact byte requirements.
- Offset Mapping: Assigns nextOffset values to variables and function parameters to define their location in the stack frame.
Symbol Table
- Type Information: Primitives, Arrays, or Structs.
- Memory Metadata: Stack offsets and total sizes.
- Function Signatures: Return types and parameter lists.

---------------------------------------------------------------------------------------------------------------------------

How to Run ( example in main.cpp )
Ensure you have a C++17 compatible compiler.

Include the Lexer, Parser, and SAnalyzer directories in your project.

Initialize the pipeline:

Lexer lexer(sourceCode);
Parser parser(lexer);
ProgramNode* ast = parser.ParseProgram();
SAnalyzer analyzer(lexer.firstToken); // Pass BaJav mode
ast->accept(&analyzer);
