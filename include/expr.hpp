#pragma once

#include "syntagma.hpp"

namespace dynaparse {
namespace expr {

struct Seq : public Expr {
	Seq(const StrIter beg, StrIter end, const Rule* r, vector<Expr*> v) :
		Expr(beg, end), nodes(new Expr*[v.size()]), size(v.size()), rule(dynamic_cast<const synt::Seq*>(r)) {
		for (uint i = 0; i < size; ++ i) nodes[i] = v[i];
	}
	virtual ~Seq() { for (uint i = 0; i < size; ++ i) delete nodes[i]; }
	Expr** nodes;
	uint   size;
	const synt::Seq* rule;
	virtual string show() const {
		if (!rule) return "null";
		string ret;
		if (rule->is_leaf) return string(beg, end);
		int i = 0;
		for (auto p : rule->right) ret += is_lexeme(p) ? p->show() : nodes[i ++]->show();
		return ret;
	}
};

/*
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

struct Alter : public Expr {
	Alter() : Expr(), node(nullptr) { }
	virtual ~Alter() { if (node) delete node; }
	Expr* node;
	virtual string show() const {
		if (!rule) return "null";
		string ret;
		if (rule->is_leaf) return string(beg, end);
		if (node) ret += node->show();
		return ret;
	}
};
*/
} // namespace expr
/*
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
*/
ostream& operator << (ostream& os, const Expr& ex) {
	os << ex.show(); return os;
}

}
