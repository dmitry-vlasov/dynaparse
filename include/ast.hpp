#pragma once
#include <expr.hpp>

namespace dynaparse {

void parse_expr(Expr& ex);
void parse_term(Expr& ex, Rule* rule);
/*
struct Rules {
	struct Node;
	typedef vector<Node> Map;
	Rule*& add(const Expr& ex);
	Map map;
};

struct Rules::Node {
	Node(Symbol s) : symb(s), tree(), level(), rule(nullptr) { }
	Symbol symb;
	Rules  tree;
	uint   level;
	Rule*  rule;
};
*/

struct Type {
	~Type();
	uint ind;
	uint id;
	vector<Type*>     sup;
	map<Type*, Rule*> supers;
	Rules             rules;
};

struct Vars {
	vector<Symbol> v;
};

struct Rule {
	uint  ind;
	uint  id;
	Type* type;
	Vars  vars;
	Expr  term;
};

inline Type::~Type() {
	for (auto p : supers) delete p.second;
}

}
