#pragma once

#include "grammar.hpp"

namespace dynaparse {

namespace rule {
struct Operator;
typedef vector<Syntagma*>::iterator OperIter;
}

struct Syntagma {
	Rule*           rule;
	rule::Operator* parent;
	rule::OperIter  place;
	Syntagma() : rule(nullptr), parent(nullptr), place() { }
	virtual ~ Syntagma() { }
	virtual string show() const = 0;
	virtual void complete(Grammar*, Rule*) = 0;
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
};

struct Operator : public Syntagma {
	vector<Syntagma*> operands;
	Operator(const vector<Syntagma*>& op) : operands(op) {
		assert(operands.size());
		for (OperIter i = operands.begin(); i != operands.end(); ++ i) {
			(*i)->place = i;
			(*i)->parent = this;
		}
	}
	virtual ~ Operator() { for (auto s : operands) delete s; }
	virtual string show() const {
		assert(operands.size());
		return show_with_delim(" ");
	}
	virtual void complete(Grammar* grammar, Rule* rule) {
		assert(operands.size());
		Syntagma::rule = rule;
		cout << "ADDING TO FLATEN: " << (void*) this << "----" << show() << endl;
		grammar->to_flaten.insert(this);
		for (auto s : operands) s->complete(grammar, rule);
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
		return 
			dynamic_cast<Seq*>(parent) ? 
			"( " + Operator::show_with_delim(" | ") + " )" : 
			Operator::show_with_delim(" | ");
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
		Syntagma* last = nullptr;
		if (!parent) {
			last = operands.back();
			operands.pop_back();
		}
		for (Syntagma* s : operands) {
			s->parent = nullptr;
			s->place = OperIter();
			rules.push_back(new Rule{new Ref(name), s});
		}
		operands.clear();
		if (last) operands.push_back(last);
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
		return {new Rule{new Ref(name), new Alt({new Ref(""), seq})}};
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
		return {new Rule{new Ref(name), new Alt({new Ref(""), seq})}};
	}
};

}

inline Syntagma* R(const string& s) { return new rule::Ref(s); }
inline Syntagma* Seq(const vector<Syntagma*>& s) { return new rule::Seq(s); }
inline Syntagma* Iter(const vector<Syntagma*>& s) { return new rule::Iter(s); }
inline Syntagma* Alt(const vector<Syntagma*>& s) { return new rule::Alt(s); }
inline Syntagma* Opt(const vector<Syntagma*>& s) { return new rule::Opt(s); }

string Rule::show() const {
	return "Rule: " + left->show() + " = " + right->show();
}
Rule::~Rule() {
	if (left) delete left;
	if (right) delete right;
}

Grammar::Grammar(const string& n) : name(n), symb_map(), symbs(), rules(), to_flaten(),
	skipper([](char c)->bool {return c <= ' '; }), c(0) {
	operator << (Keyword(""));
	//operator << (Nonterm("EMPTY"));
	//operator << (Rule(R("EMPTY"), Seq({R("")})));
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
/*
	for (Syntagma* s : to_flaten) {
		std::cout << "TO FLATTEN: " << s->show() << std::endl;
	}
	std::cout << std::endl << std::endl;
*/

	while (!to_flaten.empty()) {
		Syntagma* s = *to_flaten.begin(); 
		to_flaten.erase(to_flaten.begin());
		if (dynamic_cast<rule::Seq*>(s)) continue;
		if (s->parent) {

			std::cout << "POPPING FROM FLATEN: " << (void*) s << endl;

			std::cout << 
			"flatening: " << endl << "\t" << s->show() << endl << 
			"from:" << endl << "\t" << s->rule->show() << std::endl << std::endl;

			string nt = "N_" + std::to_string(c++);
			vector<Rule*> flat_rules = s->flaten(nt);
			symb::Nonterm* ne = new symb::Nonterm(nt);
			operator << (ne);

			//std::cout << "new nonterm: " << ne->show() << std::endl;

				rule::Ref* ref = new rule::Ref(nt);
				ref->ref = ne;
				*(s->place) = ref;
				//delete s;

			for (Rule* r : flat_rules) {
				add(r);
				//dynamic_cast<rule::Ref*>(r->left)->ref = ne;
				//rules.push_back(r);
				std::cout << "new rule:" << endl << "\t" << rules.back()->show() << std::endl;
			}

			std::cout << 
			"result:" << endl << "\t" << s->rule->show() << std::endl << std::endl;

			cout << "---------------" << endl << endl;
		} else if (rule::Alt* alt = dynamic_cast<rule::Alt*>(s)) {

			std::cout << 
			"flatening: " << endl << "\t" << s->show() << endl << 
			"from:" << endl << "\t" << s->rule->show() << std::endl << std::endl;
			//std::cout << "with parent: " << (s->parent ? s->parent->show() : "NONE") << std::endl;

			rule::Ref* nt_ref = dynamic_cast<rule::Ref*>(alt->rule->left);
			assert(nt_ref);

			vector<Rule*> flat_rules = s->flaten(nt_ref->name);
			for (Rule* r : flat_rules) {
				add(r);
				//dynamic_cast<rule::Ref*>(r->left)->ref = ne;
				//rules.push_back(r);
				std::cout << "new rule:" << endl << "\t" << rules.back()->show() << std::endl;
			}
			std::cout << 
			"result:" << endl << "\t" << s->rule->show() << std::endl << std::endl;
			cout << "---------------" << endl << endl;
		}
	}
}

}
