#include "parser.hpp"

using namespace dynaparse;

void test_grammar(Grammar& gr) {
	gr
	<< Nonterm("exp")
	<< Keyword("_(_", "(") << Keyword("_+_", "+") << Keyword("_)_", ")") << Keyword("_*_", "*")
	<< Rule("plus", "exp", {"_(_", "exp", "_+_", "exp", "_)_"})
	<< Rule("plus", "exp", {"_(_", "exp", "_*_", "exp", "_)_"})
	<< Regexp("id", "[a-zA-Z]+")
	<< Rule("ident", "exp", vector<string>{"id"});
}

void oberon_grammar(Grammar& gr) {
	gr
	<< Nonterm("exp")
	<< Keyword("(") << Keyword("+") << Keyword(")") << Keyword("*")

	<< Rule("Module", "Module", "MODULE ident ; _[ImportList]_ DeclSeq _[ BEGIN StatementSeq ]_ END ident .")
	<< Rule("ImportList", "ImportList", "IMPORT _[ ident := ]_ ident _{ , _[ ident := ]_ ident }_ ;")
//DeclSeq       = { CONST {ConstDecl ";" } | TYPE {TypeDecl ";"} | VAR {VarDecl ";"}} {ProcDecl ";" | ForwardDecl ";"}.
//ConstDecl     = IdentDef "=" ConstExpr.
//TypeDecl      = IdentDef "=" Type.
//VarDecl       = IdentList ":" Type.

	<< Rule("plus", "exp", {"(", "exp", "+", "exp", ")"})
	<< Rule("mult", "exp", {"(", "exp", "*", "exp", ")"})
	<< Regexp("id", "[a-zA-Z]+")
	<< Rule("ident", "exp", vector<string>{"id"});


	//Rule("ImportList", "IMPORT" <<  _[ ident := ]_ ident _{ , _[ ident := ]_ ident }_ ;")
}

/*   Rule("source", Seq( { node } < add_node )  )
 *   Rule("node", Seq( const | assert | include | comment ) )
 *   Rule("comment", Seq( "$(", comment-string ,"$)"))
 *   Rule("")
 *
 *
 *
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



