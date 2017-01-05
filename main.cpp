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

	string first = "(a+b)";
	if (Expr* ex = p.parse(first, "exp")) {
		std::cout << *ex << std::endl;
		std::cout << "OK" << std::endl;
	} else {
		std::cout << "FUCK!!!" << std::endl;
	}

	string sec = "   adgafgkDDFFDZ  ";
	if (Expr* ex = p.parse(sec, "exp")) {
		std::cout << *ex << std::endl;
		std::cout << "OK" << std::endl;
	} else {
		std::cout << "FUCK!!!" << std::endl;
	}

	string third = " ((  a * (xyx + bcd)) +    ( b*a))   ";
	if (Expr* ex = p.parse(third, "exp")) {
		std::cout << *ex << std::endl;
		std::cout << "OK" << std::endl;
	} else {
		std::cout << "FUCK!!!" << std::endl;
	}

	return 0;
}



