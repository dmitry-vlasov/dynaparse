#pragma once

#include "grammar.hpp"

namespace dynaparse {

namespace rule {
struct Operator;
typedef vector<Syntagma*>::iterator OperIter;
}

struct Syntagma {
	rule::Operator* parent;
	rule::OperIter  place;
	Syntagma() : parent(nullptr), place() { }
	virtual ~ Syntagma() { }
	virtual string show() const = 0;
	virtual void complete(Grammar* grammar) = 0;
	virtual vector<Rule*> flaten(const string&) { return {}; }
};

namespace rule {

//typedef Expr* (Semantic) (vector<Expr*>&);

struct Ref : public Syntagma {
	string name;
	Symb*  ref;
	Ref(const string& n) : name(n), ref(nullptr) { }
	Ref(const string& n, Symb* r) : name(n), ref(r) { }
	virtual ~ Ref() { }
	virtual string show() const { return name; }
	virtual void complete(Grammar* grammar) {
		if (ref) return;
		if (!grammar->symb_map.count(name)) {
			std::cerr << "undefined symbol: " << name << std::endl;
			throw std::exception();
		}
		ref = grammar->symb_map[name];
	}
};

struct Operator : public Syntagma {
	vector<Syntagma*> operands;
	Operator(const vector<Syntagma*>& op) : operands(op) {
		for (OperIter i = operands.begin(); i != operands.end(); ++ i) {
			(*i)->place = i;
			(*i)->parent = this;
		}
	}
	virtual ~ Operator() { for (auto s : operands) delete s; }
	virtual string show() const {
		return show_with_delim(" ");
	}
	virtual void complete(Grammar* grammar) {
		grammar->to_flaten.push(this);
		for (auto s : operands) s->complete(grammar);
	}
protected :
	string show_with_delim(const string& delim) const {
		string str;
		for (unsigned i = 0; i < operands.size(); ++ i) {
			if (i > 0) str += delim;
			str += operands[i]->show();
		}
		return str;
	}
};

struct Seq : public Operator {
	Seq(const vector<Syntagma*>& op) : Operator(op) { }
	virtual string show() const {
		return Operator::show();
	}
	/**
	 * Rule:
	 * 		M -> alpha ( beta ) gamma
	 * Transforms to:
	  *  	M -> alpha N gamma
	 *  	N -> beta
	 */
	virtual vector<Rule*> flaten(const string& name) {
		// beta:
		Seq* seq = new Seq(operands);
		operands.clear();
		return {new Rule{new Ref(name), seq}};
	}
};

struct Alt : public Operator {
	Alt(const vector<Syntagma*>& op) : Operator(op) { }
	virtual string show() const {
		return Operator::show_with_delim(" | ");
	}
	/**
	 * Rule:
	 * 		M -> alpha ( beta | gamma | delta ) epsilon
	 * Transforms to:
	 *  	M -> alpha N epsilon
	 *  	N -> beta
	 *  	N -> gamma
	 *  	N -> delta
	 */
	virtual vector<Rule*> flaten(const string& name) {
		vector<Rule*> rules;
		for (Syntagma* s : operands) {
			s->parent = nullptr;
			s->place = OperIter();
			rules.push_back(new Rule{new Ref(name), s});
		}
		operands.clear();
		return rules;
	}
};

struct Iter : public Operator {
	Iter(const vector<Syntagma*>& op) : Operator(op) { }
	virtual string show() const {
		return "{ " + Operator::show() + " }";
	}
	/**
	 * Rule:
	 * 		M -> alpha { beta } gamma
	 * Transforms to:
	 *  	M -> alpha N gamma
	 *  	N -> "" | beta N
	 */
	virtual vector<Rule*> flaten(const string& name) {
		// N:
		operands.push_back(new Ref(name));
		// beta:
		Seq* seq = new Seq(operands);
		operands.clear();
		return {new Rule{new Ref(name), new Alt({Grammar::empty_nonterm_ref(), seq})}};
	}
};

struct Opt : public Operator {
	Opt(const vector<Syntagma*>& op) : Operator(op) { }
	virtual string show() const {
		return "[ " + Operator::show() + " ]";
	}
	/**
	 * Rule:
	 * 		M -> alpha [ beta ] gamma
	 * Transforms to:
	 *  	M -> alpha N gamma
	 *  	N -> "" | beta
	 */
	virtual vector<Rule*> flaten(const string& name) {
		// beta:
		Seq* seq = new Seq(operands);
		// beta:
		operands.clear();
		return {new Rule{new Ref(name), new Alt({Grammar::empty_nonterm_ref(), seq})}};
	}
};

}

string Rule::show() const {
	return "Rule: " + left->show() + " = " + right->show();
}
Rule::~Rule() {
	if (left) delete left;
	if (right) delete right;
}

Grammar::Grammar(const string& n) : name(n), symb_map(), symbs(), rules(), to_flaten(),
	skipper([](char c)->bool {return c <= ' '; }), empty_rule(nullptr), c(0) {
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
	rules.back()->left->complete(this);
	rules.back()->right->complete(this);
}

Symb* Grammar::empty_symb() {
	static Symb* empty = new symb::Keyword("");
	return empty;
}

Symb* Grammar::empty_nonterm() {
	static Symb* empty = new symb::Nonterm("EMPTY");
	return empty;
}

Syntagma* Grammar::empty_symb_ref() {
	static Syntagma* empty = new rule::Ref("", empty_symb());
	return empty;
}

Syntagma* Grammar::empty_nonterm_ref() {
	static Syntagma* empty = new rule::Ref("EMPTY", empty_nonterm());
	return empty;
}

Grammar& Grammar::operator << (Symb* s) {
	symbs.push_back(s);
	symb_map[s->name] = s;
	if ((s == empty_symb() || s == empty_nonterm()) && !empty_rule) {
		empty_rule = new Rule(empty_nonterm_ref(), empty_symb_ref());
	}
	return *this;
}


void Grammar::flaten_ebnf() {
/*
	for (Syntagma* s : to_flaten) {
		std::cout << "TO FLATTEN: " << s->show() << std::endl;
	}
	std::cout << std::endl << std::endl;
*/

	while (!to_flaten.empty()) {
		Syntagma* s = to_flaten.front(); to_flaten.pop();
		if (dynamic_cast<rule::Seq*>(s)) continue;
		if (!s->parent && !dynamic_cast<rule::Alt*>(s)) continue;

		std::cout << "flatening: " << s->show() << std::endl;
		//std::cout << "with parent: " << (s->parent ? s->parent->show() : "NONE") << std::endl;

		string nt = "N_" + std::to_string(c++);
		vector<Rule*> flat_rules = s->flaten(nt);
		symb::Nonterm* ne = new symb::Nonterm(nt);
		operator << (ne);

		//std::cout << "new nonterm: " << ne->show() << std::endl;

		if (s->place != rule::OperIter()) {
			rule::Ref* ref = new rule::Ref(nt);
			ref->ref = ne;
			*(s->place) = ref;
			//delete s;
		}
		for (Rule* r : flat_rules) {
			add(r);
			//dynamic_cast<rule::Ref*>(r->left)->ref = ne;
			//rules.push_back(r);
			std::cout << "new rule: " << rules.back()->show() << std::endl;
		}
	}
/*
	for (Syntagma* s : to_flaten) {

		if (!s->parent && !dynamic_cast<rule::Alt*>(s)) continue;

		std::cout << "flatening: " << s->show() << std::endl;
		//std::cout << "with parent: " << (s->parent ? s->parent->show() : "NONE") << std::endl;

		string nt = "N_" + std::to_string(c++);
		vector<Rule*> flat_rules = s->flaten(nt);
		symb::Nonterm* ne = new symb::Nonterm(nt);
		operator << (ne);

		//std::cout << "new nonterm: " << ne->show() << std::endl;

		if (s->place != rule::OperIter()) {
			rule::Ref* ref = new rule::Ref(nt);
			ref->ref = ne;
			*(s->place) = ref;
			delete s;
		}
		for (Rule* r : flat_rules) {
			dynamic_cast<rule::Ref*>(r->left)->ref = ne;
			rules.push_back(r);
			std::cout << "new rule: " << rules.back()->show() << std::endl;
		}

		//std::cout << std::endl << "----------------------";
		//std::cout << this->show() << std::endl << "------------------" << std::endl;

	}
	to_flaten.clear();
*/
}

inline Syntagma* R(const string& s) { return new rule::Ref(s); }
inline Syntagma* Seq(const vector<Syntagma*>& s) { return new rule::Seq(s); }
inline Syntagma* Iter(const vector<Syntagma*>& s) { return new rule::Iter(s); }
inline Syntagma* Alt(const vector<Syntagma*>& s) { return new rule::Alt(s); }
inline Syntagma* Opt(const vector<Syntagma*>& s) { return new rule::Opt(s); }

}
