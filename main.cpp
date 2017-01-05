#include "parser.hpp"

using namespace dynaparse;

void test_grammar(Grammar& gr) {
	gr
	<< Nonterm("exp")
	<< Keyword("(") << Keyword("+") << Keyword(")") << Keyword("*")
	<< Rule("exp", {"(", "exp", "+", "exp", ")"})
	<< Rule("exp", {"(", "exp", "*", "exp", ")"})
	//<< Keyword("a") << Keyword("b")
	//<< Rule("exp", {"a"})
	//<< Rule("exp", {"b"})
	<< Regexp("id", "[a-zA-Z]+")
	<< Rule("exp", {"id"});
}

/*
Grammar smm_grammar() {
	Grammar gr;
	gr << ("exp" << Rule() << "(" << "exp" << "*" << "exp" << ")");
	gr << ("exp" << Rule() << "(" << "exp" << "+" << "exp" << ")");
	gr << ("exp" << Rule() << "a");
	gr << ("exp" << Rule() << "b");
	return gr;
}
*/

int main(int argc, const char* argv[]) {
	Grammar gr;
	test_grammar(gr);
	Parser p(gr);
	std::cout << p.getGrammar().show() << std::endl;

	if (Expr* ex = p.parse("(a+b)", "exp")) {
		std::cout << *ex << std::endl;
		std::cout << "OK" << std::endl;
	} else {
		std::cout << "FUCK!!!" << std::endl;
	}

	if (Expr* ex = p.parse("   adgafgkDDFFDZ  ", "exp")) {
		std::cout << *ex << std::endl;
		std::cout << "OK" << std::endl;
	} else {
		std::cout << "FUCK!!!" << std::endl;
	}



	if (Expr* ex = p.parse(" ((  a * b) +    ( b*a))   ", "exp")) {
		std::cout << *ex << std::endl;
		std::cout << "OK" << std::endl;
	} else {
		std::cout << "FUCK!!!" << std::endl;
	}

	return 0;
}



