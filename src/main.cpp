#include "utility.h"
#include "visitor.h"
#include "Python3Lexer.h"
#include "Python3Parser.h"
#include "antlr4-runtime.h"
#include <iostream>
using namespace antlr4;
using reverSilly::PythonInterpreter::Visitor;
// TODO: regenerating files in directory named "generated" is dangerous.
//       if you really need to regenerate,please ask TA for help.
int main(int argc, const char *argv[])
try
{
	// TODO: please don't modify the code below the construction of ifs if you want to use visitor mode
	ANTLRInputStream input(std::cin);
	Python3Lexer lexer(&input);
	CommonTokenStream tokens(&lexer);
	tokens.fill();
	Python3Parser parser(&tokens);
	tree::ParseTree *tree = parser.file_input();
	Visitor visitor;
	visitor.visit(tree);
	return 0;
}
catch(reverSilly::PythonInterpreter::ProgramError &e)
{
	std::cerr<<"error executing the program: "<<e.what()<<'\n';
	return -1;
}