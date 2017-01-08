#pragma once

#include "grammar.hpp"

namespace dynaparse {

struct Syntagma {
	virtual ~ Syntagma() { }
	virtual string show() const = 0;
	virtual void complete(Grammar* grammar) = 0;
};

namespace rule {

//typedef Expr* (Semantic) (vector<Expr*>&);

struct Ref : public Syntagma {
	string name;
	Symb*  ref;
	Ref(const string& n) : name(n), ref(nullptr) { }
	virtual ~ Ref() { }
	virtual string show() const {
		return name;
	}
	virtual void complete(Grammar* grammar) {
		if (!grammar->symb_map.count(name)) {
			std::cerr << "undefined symbol: " << name << std::endl;
			throw std::exception();
		}
		ref = grammar->symb_map[name];
	}
};

struct Operator : public Syntagma {
	vector<Syntagma*> operands;
	Operator(const vector<Syntagma*>& op) : operands(op) { }
	virtual string show() const {
		string str;
		for (auto s : operands) str += s->show() + " ";
		return str;
	}
	virtual void complete(Grammar* grammar) {
		for (auto s : operands) s->complete(grammar);
	}
};

struct Seq : public Operator {
	Seq(const vector<Syntagma*>& op) : Operator(op) { }
};

struct Iter : public Operator {
	Iter(const vector<Syntagma*>& op) : Operator(op) { }
};

struct Alt : public Operator {
	Alt(const vector<Syntagma*>& op) : Operator(op) { }
};

struct Opt : public Operator {
	Opt(const vector<Syntagma*>& op) : Operator(op) { }
};

}

string Rule::show() const {
	return left->show() + " = " + right->show();
}

Grammar& Grammar::operator << (Rule&& rule) {
	rules.push_back(rule);
	rule.left->complete(this);
	rule.right->complete(this);
	return *this;
}

inline Syntagma* R(const string& s) { return new rule::Ref(s); }
inline Syntagma* Seq(const vector<Syntagma*>& s) { return new rule::Seq(s); }
inline Syntagma* Iter(const vector<Syntagma*>& s) { return new rule::Iter(s); }
inline Syntagma* Alt(const vector<Syntagma*>& s) { return new rule::Alt(s); }
inline Syntagma* Opt(const vector<Syntagma*>& s) { return new rule::Opt(s); }

}
