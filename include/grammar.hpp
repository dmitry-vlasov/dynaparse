#pragma once

#include "std.hpp"

namespace dynaparse {
namespace xxx {

struct Symb {
	string body;
	bool   term;
	string show() const { return term ? "\"" + body + "\"" : body; }
};

struct Rule {
	string left;
	vector<Symb> right;
	Rule& operator << (const string& s) { right.push_back(Symb{s, false}); return *this; }
	string show() const {
		string ret = left + " -> ";
		for (auto& s : right) ret += s.show() + " ";
		return ret;
	}
};

inline Rule& operator << (const string& s, Rule&& r) {
	r.right.clear();
	r.left = s;
	return r;
}

struct Expr {
	Expr() : rule(nullptr), children() { }
	Expr(const Rule* r) : rule(r), children() { }
	const Rule*  rule;
	vector<Expr> children;
	string show() const {
		string ret;
		int i = 0;
		for (auto& p : rule->right) ret += p.term ? p.body : children[i ++].show();
		return ret;
	}
};

ostream& operator << (ostream& os, const Expr& ex) {
	os << ex.show(); return os;
}

struct Grammar {
	vector<Rule> rules;
	Grammar& operator << (const Rule& rule) { rules.push_back(rule); return *this; }
	string show() const {
		string ret;
		ret += "Grammar rules:\n";
		for (auto& r : rules) ret += r.show() + "\n";
		ret += "\n";
		return ret;
	}
};

}}
