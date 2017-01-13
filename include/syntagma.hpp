#pragma once

#include "grammar.hpp"

namespace dynaparse {

namespace rule {
struct Operator;
}

struct Syntagma {
	Rule*           rule;
	rule::Operator* parent;
	int             place;
	Syntagma() : rule(nullptr), parent(nullptr), place(-1) { }
	virtual ~ Syntagma() { }
	virtual string show() const = 0;
	virtual void complete(Grammar*, Rule*) = 0;
	virtual void flaten(Grammar*) { }
	virtual Syntagma* clone() const = 0;
	Syntagma* clone(map<const Syntagma*, Syntagma*>& m) const {
		Syntagma* s = clone();
		m[this] = s;
		return s;
	}
};

namespace rule {

//typedef Expr* (Semantic) (vector<Expr*>&);

struct Ref : public Syntagma {
	string name;
	Symb*  ref;
	Ref(const string& n) : name(n), ref(nullptr) { }
	Ref(Symb* r) : name(r->name), ref(r) { }
	virtual ~ Ref() { }
	virtual string show() const { return name.size() ? name : "<EMPTY>"; }
	virtual void complete(Grammar* grammar, Rule* rule) {
		if (ref) return;
		Syntagma::rule = rule;
		if (!grammar->symb_map.count(name)) {
			std::cerr << "undefined symbol: " << name << std::endl;
			throw std::exception();
		}
		ref = grammar->symb_map[name];
	}
	virtual Syntagma* clone() const { return new Ref(ref); }
};

struct Operator : public Syntagma {
	virtual int arity() const = 0;
	virtual Syntagma*& get(uint = 0) = 0;
	virtual const Syntagma* get(uint = 0) const = 0;
	virtual void insert(int i, Syntagma* s) = 0;
	virtual void insert(int i, const vector<Syntagma*>& op) = 0;
	virtual void erase(int i) = 0;
};

struct NaryOperator : public Operator {
	NaryOperator(const vector<Syntagma*>& op) : operands(op) {
		assert(operands.size());
		for (uint i = 0; i < operands.size(); ++ i) {
			Syntagma* s = operands[i];
			s->place = i;
			s->parent = this;
		}
	}
	virtual ~ NaryOperator() { for (auto s : operands) delete s; }
	virtual string show() const {
		assert(operands.size());
		return show_with_delim(" ");
	}
	virtual void complete(Grammar* grammar, Rule* rule) {
		assert(operands.size());
		Syntagma::rule = rule;
		grammar->to_flaten.insert(this);
		for (auto s : operands) s->complete(grammar, rule);
	}
	virtual int arity() const { return operands.size(); }
	virtual Syntagma*& get(uint i) {
		if (i >= operands.size()) {
			std::cerr << "wrong index " << i << " in operator of size " << operands.size() << std::endl;
			throw std::exception();
		}
		return operands[i];
	}
	virtual const Syntagma* get(uint i) const {
		if (i >= operands.size()) {
			std::cerr << "wrong index " << i << " in operator of size " << operands.size() << std::endl;
			throw std::exception();
		}
		return operands[i];
	}

	string show_with_delim(const string& delim) const {
		string str;
		for (unsigned i = 0; i < operands.size(); ++ i) {
			if (i > 0) str += delim;
			str += operands[i]->show();
		}
		return str;
	}
	virtual void insert(int i, Syntagma* s) {
		operands.insert(operands.begin() + i, s);
		s->place = i;
		s->parent = this;
	}
	virtual void insert(int i, const vector<Syntagma*>& op) {
		operands.reserve(operands.size() + op.size());
		for (Syntagma* s : op) insert(i++, s);
	}
	virtual void erase(int i) {
		operands.erase(operands.begin() + i);
	}
	vector<Syntagma*> clone_operands() const {
		vector<Syntagma*> ret;
		for (Syntagma* s : operands) ret.push_back(s->clone());
		return ret;
	}
	vector<Syntagma*> operands;
};

struct UnaryOperator : public Operator {
	UnaryOperator(Syntagma* op) : operand(op) {
		assert(operand);
	}
	virtual ~ UnaryOperator() { delete operand; }
	virtual void complete(Grammar* grammar, Rule* rule) {
		assert(operand);
		Syntagma::rule = rule;
		grammar->to_flaten.insert(this);
		operand->complete(grammar, rule);
	}
	virtual int arity() const { return 1; }
	virtual Syntagma*& get(uint i = 0) {
		if (i != 0) {
			std::cerr << "wrong index " << i << " in unary operator" << std::endl;
			throw std::exception();
		}
		return operand;
	}
	virtual const Syntagma* get(uint i = 0) const {
		if (i != 0) {
			std::cerr << "wrong index " << i << " in unary operator" << std::endl;
			throw std::exception();
		}
		return operand;
	}
	virtual void insert(int i, Syntagma* s) {
		if (i != 0) {
			std::cerr << "wrong index " << i << " in unary operator" << std::endl;
			throw std::exception();
		}
		operand = s;
	}
	virtual void insert(int i, const vector<Syntagma*>& op) {
		std::cerr << "no way to insert a vector into unary operator" << std::endl;
		throw std::exception();
	}
	virtual void erase(int i) {
		std::cerr << "no way to delete an item from unary operator" << std::endl;
		throw std::exception();
	}
	Syntagma* operand;
};

struct Seq : public NaryOperator {
	Seq(const vector<Syntagma*>& op) : NaryOperator(op) { }
	virtual string show() const {
		return NaryOperator::show();
	}
	/**
	 * Rule:
	 * 		M -> alpha (beta gamma) delta
	 * Transforms to:
	 *  	M -> alpha beta gamma delta
	 */
	virtual void flaten(Grammar* grammar) {
		if (!parent) return;
		if (Seq* seq = dynamic_cast<Seq*>(parent)) {
			seq->erase(place);
			seq->insert(place, operands);
			operands.clear();
			//delete this;
		}
	}
	virtual Syntagma* clone() const { return new Seq(clone_operands()); }
};

struct Alt : public NaryOperator {
	Alt(const vector<Syntagma*>& op) : NaryOperator(op) { }
	virtual string show() const {
		return 
			dynamic_cast<Seq*>(parent) ? 
			"( " + NaryOperator::show_with_delim(" | ") + " )" : 
			NaryOperator::show_with_delim(" | ");
	}
	/**
	 * Rule:
	 * 		M -> alpha ( beta | gamma | delta ) epsilon
	 * Transforms to:
	 *   	M -> alpha N epsilon
	 *  	N -> beta
	 *  	N -> gamma
	 *  	N -> delta
	 *
	 * Rule:
	 * 		M -> beta | gamma | delta (*)
	 * Transforms to:
	 *  	M -> beta
	 *  	M -> gamma
	 *  	M -> delta (*)
	 */
	virtual void flaten(Grammar* grammar) {
		symb::Nonterm* nt =
			parent ?
			grammar->fresh_nonterm() :
			dynamic_cast<symb::Nonterm*>(rule->left->ref);

		if (parent) {
			parent->erase(place);
			parent->insert(place, new Ref(nt));
		} else {
			rule->right = operands.back();
			operands.pop_back();
		}
		for (Syntagma* s : operands) {
			*grammar << Rule(new Ref(nt), s);
		}
		//delete this;
	}
	virtual Syntagma* clone() const { return new Alt(clone_operands()); }
};

struct Iter : public UnaryOperator {
	Iter(Syntagma* op) : UnaryOperator(op) { assert(op); }
	virtual string show() const {
		return "{ " + operand->show() + " }";
	}
	/**
	 * Rule:
	 * 		M -> alpha { beta } gamma
	 * Transforms to:
	 *  	M -> alpha N gamma
	 *  	N -> ""
	 *  	N -> beta N
	 *
	 * Rule:
	 * 		M -> { beta }
	 * Transforms to:
	 *  	M -> ""
	 *  	M -> beta M
	 */
	virtual void flaten(Grammar* grammar) {
		symb::Nonterm* nt =
			parent ?
			grammar->fresh_nonterm() :
			dynamic_cast<symb::Nonterm*>(rule->left->ref);

		if (parent) {
			parent->erase(place);
			parent->insert(place, new Ref(nt));
			Syntagma* op = operand->clone();
			Seq* seq = dynamic_cast<Seq*>(op) ? dynamic_cast<Seq*>(op) : new Seq({op});
			seq->insert(seq->arity(), new Ref(nt));
			//Rule r(new Ref(nt), seq);
			//std::cout << r.show() << std::endl;
			*grammar << Rule(new Ref(nt), seq);
		} else {
			rule->right = operand;
			if (Operator* op = dynamic_cast<Operator*>(operand)) {
				op->insert(op->arity(), new Ref(nt));
			} else {
				std::cerr << "right side of a rule cannot be ref" << std::endl;
				throw std::exception();
			}
		}
		*grammar << Rule(new Ref(nt), new Seq({new Ref("")}));

	}
	virtual Syntagma* clone() const { return new Iter(operand->clone()); }
};

struct Opt : public UnaryOperator {
	Opt(Syntagma* op) : UnaryOperator(op) { }
	virtual string show() const {
		return "[ " + operand->show() + " ]";
	}
	/**
	 * Rule:
	 * 		M -> alpha [ beta ] gamma
	 * Transforms to:
	 *  	M -> alpha N gamma
	 *  	N -> ""
	 *  	N-> beta
	 *
	 * Rule:
	 * 		M -> [ beta ]
	 * Transforms to:
	 *  	M -> ""
	 *  	M-> beta
	 *
	 */
	virtual void flaten(Grammar* grammar) {
		symb::Nonterm* nt =
			parent ?
			grammar->fresh_nonterm() :
			dynamic_cast<symb::Nonterm*>(rule->left->ref);

		if (parent) {
			parent->erase(place);
			parent->insert(place, new Ref(nt));
		} else {
			rule->right = operand;
			if (!dynamic_cast<Operator*>(operand)) {
				std::cerr << "right side of a rule cannot be ref" << std::endl;
				throw std::exception();
			}
		}
		*grammar << Rule(new Ref(nt), new Seq({new Ref("")}));
	}
	virtual void complete(Grammar* grammar, Rule* rule) {
		assert(operand);
		Syntagma::rule = rule;
		grammar->to_flaten.insert(this);
		operand->complete(grammar, rule);
	}
	virtual Syntagma* clone() const { return new Opt(operand->clone()); }
};

}

inline Syntagma* R(const string& s) { return new rule::Ref(s); }
inline Syntagma* Seq(const vector<Syntagma*>& s) { return new rule::Seq(s); }
inline Syntagma* Seq(Syntagma* s) { return new rule::Seq({s}); }
inline Syntagma* Iter(Syntagma* s) { return new rule::Iter(s); }
inline Syntagma* Iter(const vector<Syntagma*>& s) { return new rule::Iter(Seq(s)); }
inline Syntagma* Alt(Syntagma*) { std::cerr << "no sense to make alternative of 1 variant" << std::endl; throw std::exception(); return nullptr; }
inline Syntagma* Alt(const vector<Syntagma*>& s) { return new rule::Alt(s); }
inline Syntagma* Opt(Syntagma* s) { return new rule::Opt(s); }
inline Syntagma* Opt(const vector<Syntagma*>& s) { return new rule::Opt(Seq(s)); }

string Rule::show() const {
	return "Rule: " + left->show() + " = " + right->show();
}
Rule::~Rule() {
	if (left) delete left;
	if (right) delete right;
}

Rule::Rule(Syntagma* l, Syntagma* r) : left(dynamic_cast<rule::Ref*>(l)), right(r) {
	if (!left) {
		std::cerr << "left side of a rule must be a reference to non-terminal" << std::endl;
		throw std::exception();
	}
}

Rule* Rule::clone() const { return new Rule(left->clone(), right->clone());
}

Grammar::Grammar(const string& n) : name(n), symb_map(), symbs(), rules(), to_flaten(),
	skipper([](char c)->bool {return c <= ' '; }), fresh_nonterm_index(0) {
	operator << (Keyword(""));
}

Grammar& Grammar::operator << (Rule&& rule) {
	add(new Rule{rule.left, rule.right});
	rule.left = nullptr;
	rule.right = nullptr;
	return *this;
}

void Grammar::add(Rule* r) {
	rules.push_back(r);
	rules.back()->left->complete(this, r);
	rules.back()->right->complete(this, r);
}

Grammar& Grammar::operator << (Symb* s) {
	symbs.push_back(s);
	symb_map[s->name] = s;
	return *this;
}


void Grammar::flaten_ebnf() {
	while (!to_flaten.empty()) {
		Syntagma* s = *to_flaten.begin(); 

		bool show_it = false;
		if (rule::Seq* seq = dynamic_cast<rule::Seq*>(s)) {
			if (!seq->parent || !dynamic_cast<rule::Seq*>(seq->parent)) show_it = false;
		}

		if (show_it) {
			std::cout << "TO FLATEN: " << s->show() << std::endl;
			std::cout << "IN RULE: " << s->rule->show() << std::endl;
		}

		to_flaten.erase(to_flaten.begin());
		s->flaten(this);

		if (show_it) {
			std::cout << "AFTER: " << std::endl << show() << std::endl;
		}
	}
}

}
