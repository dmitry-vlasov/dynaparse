#pragma once

#include "std.hpp"

namespace dynaparse {

struct Grammar;
typedef bool (Skipper) (char);
typedef string::const_iterator StrIter;

inline void skip(Skipper* skipper, StrIter& ch, StrIter end){
	while (ch != end && skipper(*ch)) ++ch;
}

struct Symb {
	string name;
	Symb(const Symb&) = default;
	Symb(const string& n) : name(n) { }
	virtual ~ Symb() { }
	virtual bool lexeme() const = 0;
	virtual string show() const = 0;
	virtual bool matches(Skipper* skipper, StrIter& ch, StrIter end) const  = 0;
	virtual bool equals(const Symb*) const = 0;
};

struct Lexeme : public Symb {
	Lexeme(const Lexeme&) = default;
	Lexeme(const string& n) : Symb(n) { }
	virtual ~ Lexeme() { }
	virtual bool lexeme() const { return true; }
};

struct Nonterm : public Symb {
	Nonterm(const Nonterm& nt) = default;
	Nonterm(const string& n) : Symb(n) { }
	virtual ~ Nonterm() { }
	virtual bool lexeme() const { return false; }
	virtual string show() const { return name; }
	virtual bool matches(Skipper*, StrIter&, StrIter) const { return false; }
	virtual bool equals(const Symb* s) const {
		if (const Nonterm* nt = dynamic_cast<const Nonterm*>(s)) {
			return name == nt->name;
		} else return false;
	}
};

struct Keyword : public Lexeme {
	string body;
	Keyword(const Keyword& kw) = default;
	Keyword(const string& b) : Lexeme(b), body(b) { }
	Keyword(const string& n, const string& b) : Lexeme(n), body(b) { }
	virtual ~Keyword() { }
	virtual string show() const { return body; }
	virtual bool matches(Skipper* skipper, StrIter& ch, StrIter end) const {
		skip(skipper, ch, end);
		StrIter x = body.begin();
		for (; x != body.end() && ch != end; ++x, ++ch) {
			if (*x != *ch) return false;
		}
		return ch != end || x == body.end();
	}
	virtual bool equals(const Symb* s) const {
		if (const Keyword* t = dynamic_cast<const Keyword*>(s)) {
			return body == t->body;
		} else return false;
	}
private :
	void check(const string& w) {
		if (w.length() != 1) return;
		switch (w[0]) {
			case '(' :
			case ')' :
			case '[' :
			case ']' :
			case '{' :
			case '}' :
				std::cerr << "illegal keyword name: " << w << std::endl;
				throw std::exception();
			default : break;
		}
	}
};

struct Empty : public Lexeme {
	Empty(const Empty&) = default;
	Empty() : Lexeme("") { }
	virtual ~Empty() { }
	virtual string show() const { return ""; }
	virtual bool matches(Skipper* skipper, StrIter& ch, StrIter end) const {
		skip(skipper, ch, end);
		return true;
	}
	virtual bool equals(const Symb* s) const {
		return dynamic_cast<const Empty*>(s);
	}
};


struct Regexp : public Lexeme {
	string body;
	regex  regexp;
	Regexp(const Regexp& re) : Lexeme(re), body(re.body), regexp(re.regexp) { }
	Regexp(const string& n, const string& b) : Lexeme(n), body(b), regexp(b) { }
	virtual ~ Regexp() { }
	virtual string show() const { return Symb::name; }
	virtual bool matches(Skipper* skipper, StrIter& ch, StrIter end) const {
		skip(skipper, ch, end);
		std::smatch m;
		bool ret = std::regex_search(ch, end, m, regexp, std::regex_constants::match_continuous);
		if (ret) ch += m.length();
		return ret;
	}
	virtual bool equals(const Symb* s) const {
		if (const Regexp* r = dynamic_cast<const Regexp*>(s)) {
			return body == r->body;
		} else return false;
	}
};


struct Expr;
typedef Expr* (Semantic) (vector<Expr*>&);

struct Rule : public Symb {
	string         left_str;
	vector<string> right_str;
	Nonterm*       left;
	vector<Symb*>  right;
	bool           is_leaf;
	Semantic*      semantic;

	Rule(const string& name) :
		Symb(name), left_str(), right_str(), left(nullptr), right(), is_leaf(true), semantic(nullptr) { }
	Rule(const Rule&) = default;
	Rule(const string& name, const string& left, const string& right);
	Rule(const string& name, const string& left, const vector<string>& right) : Symb(name),
		left_str(left), right_str(right), left(nullptr), right(), is_leaf(true), semantic(nullptr) { }
	virtual bool lexeme() const { return false; }
	virtual string show() const {
		string ret = left->name + " = ";
		for (auto s : right) ret += s->show() + " ";
		return ret;
	}
	virtual bool matches(Skipper* skipper, StrIter& ch, StrIter end) const  {
		return false; // ???
	}
	virtual bool equals(const Symb* s) const {
		if (const Rule* r = dynamic_cast<const Rule*>(s)) {
			return name == r->name;
		} else return false;
	}
};

inline Rule& operator << (Nonterm* nt, Rule&& r) {
	r.right.clear();
	r.left = nt;
	return r;
}



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
