#pragma once

#include "grammar.hpp"

namespace dynaparse {

namespace expr {

struct Expr {
	StrIter beg;
	StrIter end;
	Expr() : beg(), end(), rule(nullptr){ }
	virtual ~Expr() {  }
	const Rule* rule;
	virtual string show() const  = 0;
};

struct Seq : public Expr {
	Seq(int sz) : Expr(), nodes(new Expr*[sz]), size(sz) { }
	virtual ~Seq() { for (int i = 0; i < size; ++ i) delete nodes[i]; }
	Expr** nodes;
	int   size;
	virtual string show() const {
		if (!rule) return "null";
		string ret;
		if (rule->is_leaf) return string(beg, end);
		int i = 0;
		for (auto p : rule->right) ret += p->lexeme() ? p->show() : nodes[i ++]->show();
		return ret;
	}
};

struct Iter : public Expr {
	Iter() : Expr(), nodes() { }
	virtual ~Iter() { for (Expr* e : nodes) delete e; }
	vector<Expr*> nodes;
	virtual string show() const {
		if (!rule) return "null";
		string ret;
		if (rule->is_leaf) return string(beg, end);
		int i = 0;
		for (auto p : rule->right) ret += p->lexeme() ? p->show() : nodes[i ++]->show();
		return ret;
	}
};

struct Choice : public Expr {
	Choice() : Expr(), node(nullptr) { }
	virtual ~Choice() { if (node) delete node; }
	Expr* node;
	virtual string show() const {
		if (!rule) return "null";
		string ret;
		if (rule->is_leaf) return string(beg, end);
		if (node) ret += node->show();
		return ret;
	}
};

}

struct Expr {
	StrIter beg;
	StrIter end;
	const Rule*   rule;
	vector<Expr*> nodes;

	Expr() : beg(), end(), rule(nullptr), nodes() { }
	virtual ~Expr() { for (Expr* e : nodes) delete e; }
	virtual string show() const {
		if (!rule) return "null";
		string ret;
		if (rule->is_leaf) return string(beg, end);
		int i = 0;
		for (auto p : rule->right) ret += p->lexeme() ? p->show() : nodes[i ++]->show();
		return ret;
	}
};

ostream& operator << (ostream& os, const Expr& ex) {
	os << ex.show(); return os;
}

}
