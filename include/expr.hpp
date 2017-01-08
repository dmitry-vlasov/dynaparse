#pragma once

#include "syntagma.hpp"

namespace dynaparse {

struct Expr {
	StrIter beg;
	StrIter end;
	Expr(StrIter b, StrIter e) : beg(b), end(e) { }
	virtual ~Expr() {  }
	virtual string show() const  = 0;
};

typedef Expr* (Semantic) (vector<Expr*>&);

namespace expr {

struct Lexeme : public Expr {
	Lexeme(StrIter b, StrIter e) : Expr(b, e) { }
	virtual ~Lexeme() {  }
	virtual string show() const { return string(beg, end); }
};

struct Operator : public Expr {
	Operator(const StrIter beg, StrIter end, const Rule* r, vector<Expr*> v) :
		Expr(beg, end), nodes(v), rule(r) { }
	virtual ~Operator() {
		for (auto n : nodes) delete n;
	}
	vector<Expr*> nodes;
	const Rule*   rule;
	virtual string show() const {
		assert(rule && "rule must be non nullptr");
		if (!rule) return "null";
		string ret;
		for (auto n : nodes) ret += n->show();
		return ret;
	}
};

struct Seq : public Operator {
	Seq(const StrIter b, StrIter e, const Rule* r, vector<Expr*> v) : Operator(b, e, r, v) { }
};

struct Iter : public Operator {
	Iter(const StrIter b, StrIter e, const Rule* r, vector<Expr*> v) : Operator(b, e, r, v) { }
};

struct Alt : public Operator {
	Alt(const StrIter b, StrIter e, const Rule* r, vector<Expr*> v) : Operator(b, e, r, v) { }
};

struct Opt : public Operator {
	Opt(const StrIter b, StrIter e, const Rule* r, vector<Expr*> v) : Operator(b, e, r, v) { }
};

} // namespace expr

ostream& operator << (ostream& os, const Expr& ex) {
	os << ex.show(); return os;
}

}
