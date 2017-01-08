#pragma once

#include "symb.hpp"

namespace dynaparse {

struct Syntagma;

struct Rule {
	Rule(Syntagma* l, Syntagma* r) : left(l), right(r) { }
	Syntagma* left;
	Syntagma* right;
	string show() const;
};

struct Grammar {
	string             name;
	map<string, Symb*> symb_map;
	vector<Symb*>      symbs;
	vector<Rule>       rules;
	Skipper*           skipper;

	Grammar& operator << (Symb* s) {
		symbs.push_back(s);
		symb_map[s->name] = s;
		return *this;
	}

	Grammar& operator << (Rule&& rule);

	Grammar& operator << (Symbs&& ss) {
		for (Symb* s : ss.symbs) operator << (s);
		return *this;
	}

	string show() const {
		string ret;
		ret += "Grammar " + name + "\n";
		ret += "--------------------\n";
		for (auto symb : symbs) ret += symb->show() + "\n";
		ret += "\n";
		for (auto& rule : rules) ret += rule.show() + "\n";
		ret += "\n";
		return ret;
	}
	Grammar(const string& n) : name(n), symb_map(), symbs(), rules(),
		skipper([](char c)->bool {return c <= ' '; }), c(0) { }
	~Grammar() { for (Symb* s : symbs) delete s; }

private :
	int c;
};

}
