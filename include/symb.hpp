#pragma once

#include "std.hpp"

namespace dynaparse {

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
	virtual string show() const = 0;
	virtual bool matches(Skipper* skipper, StrIter& ch, StrIter end) const  = 0;
	virtual bool equals(const Symb* s) const = 0;
};

struct Symbs {
	vector<Symb*> symbs;
};

namespace symb {

struct Lexeme : public Symb {
	Lexeme(const Lexeme&) = default;
	Lexeme(const string& n) : Symb(n) { }
};

struct Nonterm : public Symb {
	Nonterm(const Nonterm& nt) = default;
	Nonterm(const string& n) : Symb(n) { }
	virtual ~ Nonterm() { }
	virtual string show() const { return "Nonterm: " + name; }
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
	virtual string show() const { return "Keyword: " + body; }
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
};

struct Regexp : public Lexeme {
	string body;
	regex  regexp;
	Regexp(const Regexp& re) : Lexeme(re), body(re.body), regexp(re.regexp) { }
	Regexp(const string& n, const string& b) : Lexeme(n), body(b), regexp(b) { }
	virtual ~ Regexp() { }
	virtual string show() const { return "Regexpr: " + name + " definition: " + body; }
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

}

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
