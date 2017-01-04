#pragma once

#include "std.hpp"

namespace dynaparse {
namespace xxx {

struct Symb {
	string body;
	bool   term;
};

struct Rule {
	string left;
	vector<Symb> right;
	Rule& operator << (const string& s) { right.push_back(Symb{s, false}); return *this; }
};

inline Rule& operator << (const string& s, Rule&& r) {
	r.right.clear();
	r.left = s;
	return r;
}

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
	string show() const {
		string ret;
		int i = 0;
		if (!rule) {
			return "NULL";
		}
		for (auto& p : rule->right) {
			if (p.term) {
				ret += p.body;
			} else {
				ret += children[i ++].show();
			}
		}
		return ret;
	}
};

ostream& operator << (ostream& os, const Expr& ex) {
	os << ex.show(); return os;
}

struct Grammar {
	vector<Type> types;
	vector<Rule> rules;
	Grammar& operator << (const Rule& rule) { rules.push_back(rule); return *this; }
	Grammar& operator << (const Type& type) { types.push_back(type); return *this; }
};

}}
