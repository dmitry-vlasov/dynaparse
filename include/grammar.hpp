#pragma once

#include "symb.hpp"

namespace dynaparse {

struct Syntagma;

namespace rule {
struct Ref;
struct Operator;
}


struct Rule {
	Rule(Syntagma* l, Syntagma* r);
	~ Rule();
	rule::Ref* left;
	Syntagma*  right;
	string show() const;
	Rule* clone() const;
	Rule* clone(map<Syntagma*, Syntagma*>&) const;
};

typedef bool (Skipper) (char);

inline void skip(Skipper* skipper, StrIter& ch, StrIter end){
	while (ch != end && skipper(*ch)) ++ch;
}

struct Grammar {
	string             name;
	map<string, Symb*> symb_map;
	vector<Symb*>      symbs;
	vector<Rule*>      rules;
	set<rule::Operator*> to_flaten;
	Skipper*           skipper;
	int                fresh_nonterm_index;

	Grammar& operator << (Symb* s);
	Grammar& operator << (Rule&& rule);
	Grammar& operator << (Symbs&& ss) {
		for (Symb* s : ss.symbs) operator << (s);
		return *this;
	}

	void add(Rule* r);

	string show(bool full = true) const {
		string ret;
		if (full) {
			ret += "Grammar " + name + "\n";
			ret += "--------------------\n";
			for (auto symb : symbs) ret += symb->show() + "\n";
			ret += "\n";
		}
		for (auto rule : rules) ret += rule->show() + "\n";
		ret += "\n";
		return ret;
	}
	Grammar(const string& n);
	~Grammar() {
		for (Symb* s : symbs) delete s;
		for (Rule* r : rules) delete r;
	}

	void flaten_ebnf();

	symb::Nonterm* fresh_nonterm() {
		string nn = "N_" + std::to_string(fresh_nonterm_index++);
		symb::Nonterm* nt = new symb::Nonterm(nn);
		operator << (nt);
		return nt;
	}
};

}
