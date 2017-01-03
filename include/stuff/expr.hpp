#pragma once

#include "stuff/symbol.hpp"

namespace dynaparse {

struct Rule;
struct Expr {
	typedef vector<Expr> Children;
	enum Kind { NODE, VAR };
	union Value {
		Rule*   rule;
		Symbol* var;
	};

	Expr() : kind(VAR), val(), children() { val.var = nullptr; }
	Expr(Rule* r) : kind(NODE)  { val.rule = r; }
	Expr(Symbol& v) : kind(VAR), val(), children() { val.var = &v; }
	Expr(Rule* r, const Children& ch) : kind(NODE), val(), children(ch) {
		val.rule = r;
	}
	bool operator == (const Expr& t) const;
	bool operator != (const Expr& t) const {
		return !operator == (t);
	}

	Kind kind;
	Value val;
	Children children;
};

template<class T>
struct Tree {
	map<Rule*, vector<Tree<T>>> rules;
	map<const Symbols*, const Symbol*> entries;
};

struct Rules {
	struct Node;
	typedef vector<Node> Map;
	Rule*& add(const Symbols& ex);
	Map map;
};

struct Rules::Node {
	Node(Symbol s) : symb(s), tree(), level(), rule(nullptr) { }
	Symbol   symb;
	Rules tree;
	uint     level;
	Rule*    rule;
};

string show(const Rules& tr);

template<class T>
void add_term(Tree<T>& tree_m, const Expr& expr_t, map<const Symbol*, const Symbol*>& mp, const Symbols* ex) {
	if (expr_t.kind == Expr::VAR) {
		tree_m.entries[ex] = mp[expr_t.val.var];
		return;
	}
	if (!tree_m.rules.has(expr_t.val.rule)) {
		vector<Tree<T>>& tree_t = tree_m.rules[expr_t.val.rule];
		for_each(
			expr_t.children.begin(),
			expr_t.children.end(),
			[&tree_t](auto) mutable { tree_t.push_back(Tree<T>()); }
		);
	}
	vector<Tree<T>>& tree_t = tree_m.rules[expr_t.val.rule];
	auto tree_ch = tree_t.begin();
	for (auto& expr_ch : expr_t.children) {
		add_term(*tree_ch ++, expr_ch, mp, ex);
	}
}

} // mdl::rus
