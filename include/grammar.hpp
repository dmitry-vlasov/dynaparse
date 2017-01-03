#pragma once

#include "std.hpp"

namespace dynaparse {
namespace xxx {

struct Symb {
	static Symb T(const string& c) { return Symb {c, false}; }
	static Symb N(const string& c) { return Symb {c, true}; }
	string body;
	bool is_term;
	bool operator == (const Symb& s) const { return body == s.body && is_term == s.is_term; }
	bool operator != (const Symb& s) const { return !operator == (s); }
};

typedef vector<Symb> Symbs;

struct Rule {
	string name;
	Symb   left;
	Symbs  right;
	Rule& operator << (const Symb& s) { right.push_back(s); return *this; }
};

struct Type {
	string name;
	vector<string> sup;
	Type& operator << (const string& s) { sup.push_back(s); return *this; }
};

struct Expr {
	Expr() : rule(nullptr), children() { }
	Expr(const Rule* r) : rule(r), children() { }
	const Rule*  rule;
	vector<Expr> children;
};

struct Grammar {
	vector<Type> types;
	vector<Rule> rules;
	Grammar& operator << (const Rule& rule) { rules.push_back(rule); return *this; }
	Grammar& operator << (const Type& type) { types.push_back(type); return *this; }
};

void russell_grammar(Grammar& rus) {
	Type tp;
	Rule rul {"rule name" };
	rul << Symb::T("{") << Symb::N("N") << Symb::T("}");
	rus << tp;
}

}}
