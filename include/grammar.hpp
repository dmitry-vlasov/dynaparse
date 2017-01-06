#pragma once

#include "syntagma.hpp"

namespace dynaparse {

struct Grammar {
	map<string, Keyword*> keyword_map;
	map<string, Regexp*>  regexp_map;
	map<string, Nonterm*> nonterm_map;

	vector<Keyword*> keywords;
	vector<Regexp*>  regexps;
	vector<Nonterm*> nonterms;
	vector<Rule*>    rules;

	Skipper*     skipper;
	Grammar& operator << (const Keyword& kw) {
		keywords.push_back(new Keyword(kw));
		keyword_map[kw.name] = keywords.back();
		return *this;
	}
	Grammar& operator << (const Regexp& re) {
		regexps.push_back(new Regexp(re));
		regexp_map[re.name] = regexps.back();
		return *this;
	}
	Grammar& operator << (const Nonterm& nt) {
		nonterms.push_back(new Nonterm(nt));
		nonterm_map[nt.name] = nonterms.back();
		return *this;
	}
	Grammar& operator << (const Rule& r);
	string show() const {
		string ret;
		ret += "Grammar rules:\n";
		for (auto& r : rules) ret += r->show() + "\n";
		ret += "\n";
		return ret;
	}
	Grammar() : rules(), skipper([](char c)->bool {return c <= ' '; }), c(0) { }
	~Grammar() {
		for (Rule* r : rules) delete r;
		for (Keyword* kw : keywords) delete kw;
		for (Nonterm* nt : nonterms) delete nt;
		for (Regexp* re : regexps) delete re;
	}
private :
	int c;
	void parse_EBNF();
	string new_non_term();
	Rule* extract(Rule* r);
};

}
