#include "parser.hpp"

using namespace dynaparse;

void test_grammar(Grammar& gr) {
	gr
	<< Nonterms({"exp"})
	<< Keywords({"(", "+", ")", "*"})
	<< Regexp("id", "[a-zA-Z]+")

	<< Rule(R("exp"), Seq({R("("), R("exp"), R("+"), R("exp"), R(")")}))
	<< Rule(R("exp"), Seq({R("("), R("exp"), R("*"), R("exp"), R(")")}))
	<< Rule(R("exp"), Seq({R("id")}));
}

void oberon_grammar(Grammar& gr) {
	gr
	<< Nonterms({"Module", "ImportList", "ident", "DeclSeq", "StatementSeq", "DeclSeq", "ConstDecl", "TypeDecl", "VarDecl", "Type"})
	<< Nonterms({"ConstExpr", "ProcDecl", "ForwardDecl", "IdentDef", "IdentList"})
	<< Keywords({"(", "MODULE", "BEGIN", "END", ";", ".", ":", "CONST", "TYPE", "VAR", "IMPORT", ":=",  ",", "="})

	<< Rule(R("Module"), Seq({
	R("MODULE"), R("ident"), R(";"), Opt({R("ImportList")}), R("DeclSeq"), Opt({R("BEGIN"), R("StatementSeq")}), R("END"), R("ident"), R(".")
	}))
	<< Rule(R("ImportList"), Seq({
	R("IMPORT"), Opt({R("ident"), R(":=")}), R("ident"), Iter({R(","), Opt({R("ident"), R(":=")}), R("ident")}), R(";")
	}))
	<< Rule(R("DeclSeq"), Seq({
		Iter(
			{Alt({
				Seq({R("CONST"), Iter({R("ConstDecl"), R(";")})
				}),
				Seq({R("TYPE"), Iter({R("TypeDecl"), R(";")})
				}),
				Seq({R("VAR"), Iter({R("VarDecl"), R(";")})
				}),
			})}
		),
		Iter(
			{Alt({
				Seq({R("ProcDecl"), R(";")}),
				Seq({R("ForwardDecl"), R(";")})
			})}
		)
	}))
	<< Rule(R("ConstDecl"), Seq({R("IdentDef"), R("="), R("ConstExpr")}))
	<< Rule(R("TypeDecl"), Seq({R("IdentDef"), R("="), R("Type")}))
	<< Rule(R("VarDecl"), Seq({R("IdentList"), R(":"), R("Type")}));
}


void test_1() {
	Grammar gr("test");
	test_grammar(gr);
	gr.flaten_ebnf();
	Parser p(gr);
	std::cout << p.getGrammar().show() << std::endl;

	string first = "(a+b)";
	if (Expr* ex = p.parse(first, "exp")) {
		std::cout << *ex << std::endl;
		std::cout << "OK" << std::endl;
		delete ex;
	} else {
		std::cout << "FUCK!!!" << std::endl;
	}

	string sec = "   adgafgkDDFFDZ  ";
	if (Expr* ex = p.parse(sec, "exp")) {
		std::cout << *ex << std::endl;
		std::cout << "OK" << std::endl;
		delete ex;
	} else {
		std::cout << "FUCK!!!" << std::endl;
	}

	string third = " ((  a * (xyx + bcd)) +    ( b*a))   ";
	if (Expr* ex = p.parse(third, "exp")) {
		std::cout << *ex << std::endl;
		std::cout << "OK" << std::endl;
		delete ex;
	} else {
		std::cout << "FUCK!!!" << std::endl;
	}
}

void test_ober() {
	Grammar gr("oberon");
	oberon_grammar(gr);
	std::cout << gr.show() << std::endl;
	gr.flaten_ebnf();
	//Parser p(gr);
	std::cout << gr.show() << std::endl;
}

int main(int argc, const char* argv[]) {
	test_1();
	test_ober();
	return 0;
}



