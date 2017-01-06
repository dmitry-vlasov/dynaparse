#pragma once

#include "std.hpp"

namespace dynaparse {

typedef bool (Skipper) (char);
typedef string::const_iterator StrIter;

inline void skip(Skipper* skipper, StrIter& ch, StrIter end){
	while (ch != end && skipper(*ch)) ++ch;
}

struct Expr {
	StrIter beg;
	StrIter end;
	Expr() : beg(), end() { }
	Expr(StrIter b, StrIter e) : beg(b), end(e) { }
	virtual ~Expr() {  }
	virtual string show() const  = 0;
};

typedef Expr* (Semantic) (vector<Expr*>&);

struct Grammar;

struct Syntagma {
	string name;
	Syntagma(const Syntagma&) = default;
	Syntagma(const string& n) : name(n) { }
	virtual ~ Syntagma() { }
	virtual string show() const = 0;
	virtual bool matches(Skipper* skipper, StrIter& ch, StrIter end) const  = 0;
	virtual bool equals(const Syntagma*) const = 0;
	virtual void complete(Grammar*) { }
};

struct Grammar {
	map<string, Syntagma*> synt_map;
	vector<Syntagma*>      syntagmas;
	Skipper*               skipper;

	Grammar& operator << (Syntagma* s) {
		syntagmas.push_back(s);
		synt_map[s->name] = s;
		s->complete(this);
		return *this;
	}

	string show() const {
		string ret;
		ret += "Grammar syntagmas:\n";
		for (auto s : syntagmas) ret += s->show() + "\n";
		ret += "\n";
		return ret;
	}
	Grammar() : synt_map(), syntagmas(), skipper([](char c)->bool {return c <= ' '; }), c(0) { }
	~Grammar() { for (Syntagma* s : syntagmas) delete s; }

private :
	int c;
};

}
