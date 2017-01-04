//#include <stuff/parse.hpp>
#include "parser.hpp"

using namespace dynaparse;
using namespace xxx;

void test_grammar(Grammar& gr) {
	gr << ("exp" << Rule() << "(" << "exp" << "+" << "exp" << ")");
	gr << ("exp" << Rule() << "(" << "exp" << "*" << "exp" << ")");
	gr << ("exp" << Rule() << "a");
	gr << ("exp" << Rule() << "b");
}


int main(int argc, const char* argv[]) {
	Grammar gr;
	test_grammar(gr);
	Parser p(gr);
	Expr ex;
	p.parse("(a+(b*a))", ex, "exp");
	std::cout << ex << std::endl;
	return 0;
}



