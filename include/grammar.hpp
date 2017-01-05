#pragma once

#include "std.hpp"

namespace dynaparse {

typedef bool (Skipper) (char);
typedef string::const_iterator StrIter;

struct Symb {
	string body;
	bool   term;
	string show() const { return term ? "\"" + body + "\"" : body; }
	bool matches(Skipper* skipper, StrIter ch, StrIter end) const {
		assert(term);
		while (ch != end && skipper(*ch)) ++ch;
		StrIter x = body.begin();
		for (; x != body.end() && ch != end; ++x, ++ch) {
			if (*x != *ch) return false;
		}
		return ch != end || x == body.end();
	}
};

struct Expr;

typedef void (Semantic) (Expr*, StrIter beg, StrIter end);

struct Rule {
	string       left;
	vector<Symb> right;
	Semantic*    semantic;
	Rule() : left(), right(), semantic(nullptr) { }
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
	Expr() : rule(nullptr), nodes() { }
	virtual ~Expr() { for (Expr* e : nodes) delete e; }
	const Rule*   rule;
	vector<Expr*> nodes;
	string show() const {
		if (!rule) return "null";
		string ret;
		int i = 0;
		for (auto& p : rule->right) ret += p.term ? p.body : nodes[i ++]->show();
		return ret;
	}
};

ostream& operator << (ostream& os, const Expr& ex) {
	os << ex.show(); return os;
}

struct Grammar {
	vector<Rule> rules;
	Skipper*     skipper;
	Grammar& operator << (const Rule& rule) { rules.push_back(rule); return *this; }
	string show() const {
		string ret;
		ret += "Grammar rules:\n";
		for (auto& r : rules) ret += r.show() + "\n";
		ret += "\n";
		return ret;
	}
	Grammar() : rules(), skipper([](char c)->bool {return c <= ' '; }) { }
};

}
