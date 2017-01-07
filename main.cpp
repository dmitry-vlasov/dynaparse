#include "parser.hpp"

using namespace dynaparse;

void test_grammar(Grammar& gr) {
	gr
	<< Nonterm("exp")
	<< Keywords({"(", "+", ")", "*"})
	<< Seq("plus", "exp", {"(", "exp", "+", "exp", ")"})
	<< Seq("plus", "exp", {"(", "exp", "*", "exp", ")"})
	<< Regexp("id", "[a-zA-Z]+")
	<< Seq("ident", "exp", vector<string>{"id"});


	//gr << new synt::Seq("AAA", "exp", {new synt::Seq("AA"), new Keyword("(")});

	/*
	gr << new synt::Seq("ImportList", "ImportList",
		{
			new Keyword("IMPORT"),
			new synt::Opt(
				{
					new Regexp("ident", ""),
					new Keyword(":=")
				}
			),
			new Regexp("ident", ""),
			new synt::Iter(
				{
					new Keyword(","),
					new synt::Opt(
						{
							new Regexp("ident", ""),
							new Keyword(":=")
						}
					),
					new Regexp("ident", "")
				}
			),
			new Keyword(";")
		}
	);
	*/
}

void oberon_grammar(Grammar& gr) {
	gr
	<< Nonterms({"Module", "ImportList", "ident", "DeclSeq", "StatementSeq", "DeclSeq", "ConstDecl", "TypeDecl", "VarDecl", "Type"})
	<< Nonterms({"ConstExpr", "ProcDecl", "ForwardDecl"})
	<< Keywords({"(", "MODULE", "BEGIN", "END", ";", ".", ":", "CONST", "TYPE", "VAR"})
	<< Seq("Module", "Module", {
	KW("MODULE"), NT("ident"), KW(";"), Opt({NT("ImportList")}), NT("DeclSeq"), Opt({KW("BEGIN"), NT("StatementSeq")}), KW("END"), NT("ident"), KW(".")
	})
	<< Seq("ImportList", "ImportList", {
	KW("IMPORT"), Opt({NT("ident"), KW(":=")}), NT("ident"), Iter({KW(","), Opt({NT("ident"), KW(":=")}), NT("ident")}), KW(";")
	})
	<< Seq({
		Iter(
			{Alt({
				Seq({KW("CONST"), Iter({NT("ConstDecl"), KW(";")})
				}),
				Seq({KW("TYPE"), Iter({NT("TypeDecl"), KW(";")})
				}),
				Seq({KW("VAR"), Iter({NT("VarDecl"), KW(";")})
				}),
			})}
		),
		Iter(
			{Alt({
				Seq({NT("ProcDecl"), KW(";")}),
				Seq({NT("ForwardDecl"), KW(";")})
			})}
		)
	})
	<< Seq("ConstDecl", "ConstDecl", {NT("IdentDef"), KW("="), NT("ConstExpr")})
	<< Seq("TypeDecl", "TypeDecl", {NT("IdentDef"), KW("="), NT("Type")})
	<< Seq("VarDecl", "VarDecl", {NT("IdentList"), KW(":"), NT("Type")});
}


void test_1() {
	Grammar gr("test");
	test_grammar(gr);
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

int main(int argc, const char* argv[]) {
	test_1();
	return 0;
}



