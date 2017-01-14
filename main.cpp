#include "parser.hpp"

using namespace dynaparse;

void test_1_grammar(Grammar& gr) {
	gr
	<< Nonterms({"exp"})
	<< Keywords({"(", "+", ")", "*"})
	<< Regexp("id", "[a-zA-Z]+")

	<< Rule(R("exp"), Seq({R("("), R("exp"), R("+"), R("exp"), R(")")}))
	<< Rule(R("exp"), Seq({R("("), R("exp"), R("*"), R("exp"), R(")")}))
	<< Rule(R("exp"), Seq({R("id")}));
}

void test_2_grammar(Grammar& gr) {
	gr
	<< Nonterms({"A", "B", "C"})
	<< Keywords({"a", "b", "c"})
	<< Rule(R("A"), Iter(Alt({R("a"), R("b")})))
	;
}

void oberon_grammar(Grammar& gr) {
	gr
	<< Nonterms({"Module", "ImportList", "ident", "DeclSeq", "StatementSeq", "DeclSeq", "ConstDecl", "TypeDecl", "VarDecl", "Type"})
	<< Nonterms({"ConstExpr", "ProcDecl", "ForwardDecl", "IdentDef", "IdentList"})
	<< Keywords({"(", "MODULE", "BEGIN", "END", ";", ".", ":", "CONST", "TYPE", "VAR", "IMPORT", ":=",  ",", "="})

	<< Rule(R("Module"), Seq({
		R("MODULE"),
		R("ident"),
		R(";"),
		Opt(R("ImportList")),
		R("DeclSeq"),
		Opt({R("BEGIN"), R("StatementSeq")}),
		R("END"),
		R("ident"),
		R(".")
	}))
	<< Rule(R("ImportList"), Seq({
		R("IMPORT"),
		Opt({R("ident"), R(":=")}),
		R("ident"),
		Iter({R(","), Opt({R("ident"), R(":=")}), R("ident")}),
		R(";")
	}))
	<< Rule(R("DeclSeq"), Seq({
		Iter(
			Alt({
				Seq({R("CONST"), Iter({R("ConstDecl"), R(";")})}),
				Seq({R("TYPE"), Iter({R("TypeDecl"), R(";")})}),
				Seq({R("VAR"), Iter({R("VarDecl"), R(";")})}),
			})
		),
		Iter(
			Alt({
				Seq({R("ProcDecl"), R(";")}),
				Seq({R("ForwardDecl"), R(";")})
			})
		)
	}))
	<< Rule(R("ConstDecl"), Seq({R("IdentDef"), R("="), R("ConstExpr")}))
	<< Rule(R("TypeDecl"), Seq({R("IdentDef"), R("="), R("Type")}))
	<< Rule(R("VarDecl"), Seq({R("IdentList"), R(":"), R("Type")}));
}

void make_test(Parser& p, const string& s, const string& nt) {
	string str = s;
	if (Expr* ex = p.parse(str, nt)) {
		std::cout << *ex << std::endl;
		std::cout << "OK" << std::endl;
		delete ex;
	} else {
		std::cout << "FUCK!!! : " << str << std::endl;
	}
}


void test_1() {
	Grammar gr("test_1");
	test_1_grammar(gr);
	gr.flaten_ebnf();
	Parser p(gr);
	std::cout << gr.show() << std::endl;
	make_test(p, "(a+b)", "exp");
	make_test(p, "   adgafgkDDFFDZ  ", "exp");
	make_test(p, " ((  a * (xyx + bcd)) +    ( b*a))   ", "exp");
}

void test_2() {
	Grammar gr("test_2");
	test_2_grammar(gr);
	std::cout << gr.show() << std::endl;
	gr.flaten_ebnf();
	//std::cout << gr.show() << std::endl;
	Parser p(gr);
	std::cout << gr.show() << std::endl;
	std::cout << show(p) << std::endl;

	//make_test(p, "", "A");
	make_test(p, "a", "A");
	make_test(p, "b", "A");
	make_test(p, "ab", "A");
	make_test(p, "aab", "A");
	make_test(p, "bababbaaa", "A");
}

void test_ober() {
	Grammar gr("oberon");
	oberon_grammar(gr);
	std::cout << gr.show() << std::endl;
	gr.flaten_ebnf();
	std::cout << gr.show() << std::endl;
	Parser p(gr);
	std::cout << gr.show() << std::endl;
}

int main(int argc, const char* argv[]) {
	//test_1();
	test_2();
	test_ober();
	std::cout << "SUCCESS" << std::endl;
	return 0;
}



