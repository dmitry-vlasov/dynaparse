#include "parser.hpp"

using namespace dynaparse;

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
	std::cout << gr.show() << std::endl;

	Expr ex;
	if (!p.parse(" ((  a * b) +    ( b*a))   ", ex, "exp")) {
		std::cout << "FUCK!" << std::endl;
	} else {
		std::cout << "OK" << std::endl;
	}
	std::cout << ex << std::endl;
	return 0;
}



