#include "parser.hpp"

using namespace dynaparse;

Grammar test_grammar() {
	Grammar gr;
	gr << ("exp" << Rule() << "(" << "exp" << "*" << "exp" << ")");
	gr << ("exp" << Rule() << "(" << "exp" << "+" << "exp" << ")");
	gr << ("exp" << Rule() << "a");
	gr << ("exp" << Rule() << "b");
	return gr;
}

Grammar smm_grammar() {
	Grammar gr;
	gr << ("exp" << Rule() << "(" << "exp" << "*" << "exp" << ")");
	gr << ("exp" << Rule() << "(" << "exp" << "+" << "exp" << ")");
	gr << ("exp" << Rule() << "a");
	gr << ("exp" << Rule() << "b");
	return gr;
}


int main(int argc, const char* argv[]) {
	Parser p(test_grammar());
	std::cout << p.getGrammar().show() << std::endl;

	if (Expr* ex = p.parse(" ((  a * b) +    ( b*a))   ", "exp")) {
		std::cout << *ex << std::endl;
		std::cout << "OK" << std::endl;
	} else {
		std::cout << "FUCK!!!" << std::endl;
	}

	return 0;
}



