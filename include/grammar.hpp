#pragma once

#include "symb.hpp"

namespace dynaparse {

struct Syntagma;

struct Rule {
	Rule(Syntagma* l, Syntagma* r) : left(l), right(r) { }
	~ Rule();
	Syntagma* left;
	Syntagma* right;
	string show() const;
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
	set<Syntagma*>     to_flaten;
	Skipper*           skipper;

	Grammar& operator << (Symb* s);
	Grammar& operator << (Rule&& rule);
	Grammar& operator << (Symbs&& ss) {
		for (Symb* s : ss.symbs) operator << (s);
		return *this;
	}

	void add(Rule* r);

	string show() const {
		string ret;
		ret += "Grammar " + name + "\n";
		ret += "--------------------\n";
		for (auto symb : symbs) ret += symb->show() + "\n";
		ret += "\n";
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

private :
	int c;
};

inline Symb* Keyword(const string& n) { return new symb::Keyword(n); }
inline Symb* Keyword(const string& n, const string& b) { return new symb::Keyword(n, b); }
inline Symb* Nonterm(const string& n) { return new symb::Nonterm(n); }
inline Symb* Regexp(const string& n, const string& b) { return new symb::Regexp(n, b); }

struct Nonterms : public Symbs {
	Nonterms(const vector<string>& nt) {
		for (const string& s : nt) symbs.push_back(new symb::Nonterm(s));
	}
};

struct Keywords : public Symbs {
	Keywords(const vector<string>& kws) {
		for (const string& kw : kws) symbs.push_back(new symb::Keyword(kw));
	}
};

}
