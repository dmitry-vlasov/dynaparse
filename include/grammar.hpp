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

struct Type {
	string name;
	vector<string> sup;
	Type& operator << (const string& s) { sup.push_back(s); return *this; }
	string show() const {
		if (!sup.size()) return name;
		string ret = name;
		ret += " : ";
		for (auto& s : sup) ret += s + " ";
		return ret;
	}
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
	string show() const {
		string ret;
		ret += "Non-terminals:\n";
		for (auto& t : types) ret += t.show() + "\n";
		ret += "\n";
		ret += "Rules:\n";
		for (auto& r : rules) ret += r.show() + "\n";
		ret += "\n";
		return ret;
	}
};

}}
