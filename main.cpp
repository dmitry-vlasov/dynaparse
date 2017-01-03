//#include <stuff/parse.hpp>
#include "parser.hpp"

using namespace dynaparse;
using namespace xxx;

void test_grammar(Grammar& gr) {
	gr << Type {"exp"} << Type {"int"};
	gr << ("exp" << Rule() << "(" << "exp" << ")");
	gr << ("exp" << Rule() << "exp" << "+" << "exp");
	gr << ("exp" << Rule() << "exp" << "*" << "exp");
	gr << ("exp" << Rule() << "exp" << "+" << "exp");
	gr << ("exp" << Rule() << "int");
	gr << ("int" << Rule() << "0" << "int");
	gr << ("int" << Rule() << "1" << "int");
	gr << ("int" << Rule() << "0");
	gr << ("int" << Rule() << "1");
}


int main(int argc, const char* argv[]) {
	Grammar gr;
	test_grammar(gr);
	Parser p(gr);
	Expr ex;
	p.parse("10 + (01 * 1010)", ex, "exp");
	std::cout << ex << std::endl;
	return 0;
}



