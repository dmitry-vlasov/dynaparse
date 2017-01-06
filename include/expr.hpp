#pragma once

#include "syntagma.hpp"

namespace dynaparse {
namespace expr {

struct Seq : public Expr {
	Seq(const StrIter beg, StrIter end, const Rule* r, vector<Expr*> v) :
		Expr(beg, end), nodes(new Expr*[v.size()]), size(v.size()), rule(dynamic_cast<const synt::Seq*>(r)) {
		for (int i = 0; i < size; ++ i) nodes[i] = v[i];
	}
	virtual ~Seq() { for (int i = 0; i < size; ++ i) delete nodes[i]; }
	Expr** nodes;
	int    size;
	const synt::Seq* rule;
	virtual string show() const {
		if (!rule) return "null";
		string ret;
		if (rule->is_leaf) {
			return string(beg, end);
		}
		int i = 0;
		for (auto p : rule->right)
			if (const Keyword* kw = dynamic_cast<const Keyword*>(p)) {
				ret += kw->body;
			} else if (const Regexp* re = dynamic_cast<const Regexp*>(p)) {
				ret += re->name;
			} else {
				ret += nodes[i ++]->show();
			}
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

ostream& operator << (ostream& os, const Expr& ex) {
	os << ex.show(); return os;
}

}
