#pragma once

#include "std.hpp"

namespace dynaparse {

typedef bool (Skipper) (char);
typedef string::const_iterator StrIter;

inline void skip(Skipper* skipper, StrIter& ch, StrIter end){
	while (ch != end && skipper(*ch)) ++ch;
}

struct Expr;
typedef Expr* (Semantic) (vector<Expr*>&);

namespace synt {

struct Syntagma {
	string name;
	Syntagma(const Syntagma&) = default;
	Syntagma(const string& n) : name(n) { }
	virtual ~ Syntagma() { }
	virtual string show() const = 0;
	virtual bool matches(Skipper* skipper, StrIter& ch, StrIter end) const  = 0;
	virtual bool equals(const Syntagma*) const = 0;

	bool lexeme() const;
	bool nonterm() const;
	bool rule() const;
};

struct Lexeme : public Syntagma {
	Lexeme(const Lexeme&) = default;
	Lexeme(const string& n) : Syntagma(n) { }
};

bool Syntagma::lexeme() const { return dynamic_cast<const Lexeme*>(this); }

struct Nonterm : public Syntagma {
	Nonterm(const Nonterm& nt) = default;
	Nonterm(const string& n) : Syntagma(n) { }
	virtual ~ Nonterm() { }
	virtual string show() const { return name; }
	virtual bool matches(Skipper*, StrIter&, StrIter) const { return false; }
	virtual bool equals(const Syntagma* s) const {
		if (const Nonterm* nt = dynamic_cast<const Nonterm*>(s)) {
			return name == nt->name;
		} else return false;
	}
};

bool Syntagma::nonterm() const { return dynamic_cast<const Nonterm*>(this); }

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
	virtual bool equals(const Syntagma* s) const {
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
	virtual bool equals(const Syntagma* s) const {
		return dynamic_cast<const Empty*>(s);
	}
};


struct Regexp : public Lexeme {
	string body;
	regex  regexp;
	Regexp(const Regexp& re) : Lexeme(re), body(re.body), regexp(re.regexp) { }
	Regexp(const string& n, const string& b) : Lexeme(n), body(b), regexp(b) { }
	virtual ~ Regexp() { }
	virtual string show() const { return Syntagma::name; }
	virtual bool matches(Skipper* skipper, StrIter& ch, StrIter end) const {
		skip(skipper, ch, end);
		std::smatch m;
		bool ret = std::regex_search(ch, end, m, regexp, std::regex_constants::match_continuous);
		if (ret) ch += m.length();
		return ret;
	}
	virtual bool equals(const Syntagma* s) const {
		if (const Regexp* r = dynamic_cast<const Regexp*>(s)) {
			return body == r->body;
		} else return false;
	}
};

struct Rule : public Syntagma {
	string         left_str;
	Nonterm*       left;
	Semantic*      semantic;

	Rule(const string& name) :
		Syntagma(name), left_str(), left(nullptr), semantic(nullptr) { }
	Rule(const Rule&) = default;
	Rule(const string& name, const string& left) : Syntagma(name),
		left_str(left), left(nullptr), semantic(nullptr) { }
	virtual bool lexeme() const { return false; }
	virtual string show() const = 0;
	virtual bool matches(Skipper* skipper, StrIter& ch, StrIter end) const = 0;
	virtual bool equals(const Syntagma* s) const = 0;
};

bool Syntagma::rule() const { return dynamic_cast<const Rule*>(this); }

struct Seq : public Rule {
	vector<string> right_str;
	vector<Syntagma*> right;
	bool is_leaf;

	Seq(const string& name) :
		Rule(name), right_str(), right(), is_leaf(true) { }
	Seq(const Seq&) = default;
	Seq(const string& name, const string& left, const string& right) : Rule(name, left),
			right_str(), right(), is_leaf(true) {
		std::stringstream ss(right);
		std::string item;
		while (std::getline(ss, item, ' ')) {
			right_str.push_back(item);
		}
	}
	Seq(const string& name, const string& left, const vector<string>& right) : Rule(name, left),
		right_str(right), right(), is_leaf(true) { }
	virtual string show() const {
		string ret = left->name + " = ";
		for (auto s : right) ret += s->show() + " ";
		return ret;
	}
	virtual bool matches(Skipper* skipper, StrIter& ch, StrIter end) const  {
		return false; // ???
	}
	virtual bool equals(const Syntagma* s) const {
		if (const Seq* sq = dynamic_cast<const Seq*>(s)) {
			return name == sq->name;
		} else return false;
	}
};



} // namespace synt

typedef synt::Syntagma Syntagma;
typedef synt::Lexeme Lexeme;
typedef synt::Keyword Keyword;
typedef synt::Regexp Regexp;
typedef synt::Nonterm Nonterm;
typedef synt::Empty Empty;
typedef synt::Seq Rule;


inline Rule& operator << (Nonterm* nt, Rule&& r) {
	r.right.clear();
	r.left = nt;
	return r;
}

}
