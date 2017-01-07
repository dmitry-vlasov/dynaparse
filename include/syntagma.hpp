#pragma once

#include "grammar.hpp"

namespace dynaparse {
namespace rule {

struct Lexeme : public Syntagma {
	Lexeme(const Lexeme&) = default;
	Lexeme(const string& n) : Syntagma(n) { }
};

struct Nonterm : public Syntagma {
	Nonterm(const Nonterm& nt) = default;
	Nonterm(const string& n) : Syntagma(n) { }
	virtual ~ Nonterm() { }
	virtual string show_def() const { return "Nonterm: " + name; }
	virtual bool matches(Skipper*, StrIter&, StrIter) const { return false; }
	virtual bool equals(const Syntagma* s) const {
		if (const Nonterm* nt = dynamic_cast<const Nonterm*>(s)) {
			return name == nt->name;
		} else return false;
	}
	virtual Syntagma* clone() const {
		return new Nonterm(*this);
	}
};

struct Nonterms : public Syntagmas {
	Nonterms(const vector<string>& nt) {
		for (const string& s : nt) syntagmas.push_back(new Nonterm(s));
	}
};

struct Keyword : public Lexeme {
	string body;
	Keyword(const Keyword& kw) = default;
	Keyword(const string& b) : Lexeme(b), body(b) { }
	Keyword(const string& n, const string& b) : Lexeme(n), body(b) { }
	virtual ~Keyword() { }
	virtual string show_def() const { return "Keyword: " + body; }
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
	virtual Syntagma* clone() const {
		return new Keyword(*this);
	}
};

struct Keywords : public Syntagmas {
	Keywords(const vector<string>& kws) {
		for (const string& kw : kws) syntagmas.push_back(new Keyword(kw));
	}
};

struct Empty : public Lexeme {
	Empty(const Empty&) = default;
	Empty() : Lexeme("") { }
	virtual ~Empty() { }
	virtual string show_def() const { return "Empty"; }
	virtual bool matches(Skipper* skipper, StrIter& ch, StrIter end) const {
		skip(skipper, ch, end);
		return true;
	}
	virtual bool equals(const Syntagma* s) const {
		return dynamic_cast<const Empty*>(s);
	}
	virtual Syntagma* clone() const {
		return new Empty(*this);
	}
};


struct Regexp : public Lexeme {
	string body;
	regex  regexp;
	Regexp(const Regexp& re) : Lexeme(re), body(re.body), regexp(re.regexp) { }
	Regexp(const string& n, const string& b) : Lexeme(n), body(b), regexp(b) { }
	virtual ~ Regexp() { }
	virtual string show_def() const { return "Regexpr: " + Syntagma::name + " definition: " + body; }
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
	virtual Syntagma* clone() const {
		return new Regexp(*this);
	}
};

struct Rule : public Syntagma {
	string         left_str;
	Nonterm*       left;
	vector<string> right_str;
	vector<Syntagma*> right;
	bool is_leaf;
	Semantic*      semantic;

	Rule(const string& name = "") : Syntagma(name),
		left_str(), left(nullptr), right_str(), right(), is_leaf(true), semantic(nullptr) { }
	Rule(const string& name, const string& left) : Syntagma(name),
		left_str(left), left(nullptr), right_str(), right(), is_leaf(true), semantic(nullptr) { }
	Rule(const Rule&) = default;
	Rule(const string& name, const string& left, const string& right) : Syntagma(name),
		left_str(left), left(nullptr), right_str(), right(), is_leaf(true), semantic(nullptr) {
		std::stringstream ss(right);
		std::string item;
		while (std::getline(ss, item, ' ')) {
			right_str.push_back(item);
		}
	}
	Rule(const string& name, const string& left, const vector<string>& right) : Syntagma(name),
		left_str(left), left(nullptr), right_str(right), right(), is_leaf(true), semantic(nullptr) { }

	Rule(const string& name, const string& left, const vector<Syntagma*>& r) : Syntagma(name),
		left_str(left), left(nullptr), right_str(), right(r), is_leaf(true), semantic(nullptr) { }

	Rule(const vector<Syntagma*>& r) : Syntagma(name),
		left_str(), left(nullptr), right_str(), right(r), is_leaf(true), semantic(nullptr) { }

	virtual bool lexeme() const { return false; }
	virtual bool matches(Skipper* skipper, StrIter& ch, StrIter end) const = 0;
	virtual bool equals(const Syntagma* s) const = 0;
	virtual void complete(Grammar* grammar) {
		if (!grammar->synt_map.count(left_str)) {
			std::cerr << "undefined non-terminal: " << left_str << std::endl;
			throw std::exception();
		}
		if (Nonterm* nt = dynamic_cast<Nonterm*>(grammar->synt_map[left_str])) {
			left = nt;
		} else {
			std::cerr << "symbol " << left_str << " is not non-terminal: " << std::endl;
			throw std::exception();
		}
		for (string& s : right_str) {
			if (grammar->synt_map.count(s)) {
				Syntagma* ss = grammar->synt_map[s];
				right.push_back(ss);
				if (dynamic_cast<Nonterm*>(ss)) {
					is_leaf = false;
				}
			} else {
				std::cerr << "undefined symbol: " << s << std::endl;
				throw std::exception();
			}
		}
	}
};

struct Seq : public Rule {
	//Seq(const string& name) :
	//	Rule(name) { }
	Seq(const Seq&) = default;
	Seq(const string& name, const string& left, const string& right) : Rule(name, left, right) { }
	Seq(const string& name, const string& left, const vector<string>& right) : Rule(name, left, right) { }
	Seq(const string& name, const string& left, const vector<Syntagma*>& right) : Rule(name, left, right) { }
	Seq(const vector<Syntagma*>& right) : Rule(right) { }
	virtual string show_def() const {
		string ret = "Sequental rule: " + left->name + " = ";
		for (auto s : right) ret += s->show_ref() + " ";
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
	virtual Syntagma* clone() const {
		return new Seq(*this);
	}
};

struct Iter : public Rule {
	//Seq(const string& name) :
	//	Rule(name) { }
	Iter(const Iter&) = default;
	Iter(const string& name, const string& left, const string& right) :
		Rule(name, left, vector<string>{right}) { }
	Iter(const string& name, const string& left, const vector<string>& right) : Rule(name, left, right) { }
	Iter(const string& name, const string& left, const vector<Syntagma*>& right) : Rule(name, left, right) { }
	Iter(const vector<Syntagma*>& right) : Rule(right) { }
	virtual string show_def() const {
		string ret = "Sequental rule: " + left->name + " = ";
		for (auto s : right) ret += s->show_ref() + " ";
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
	virtual Syntagma* clone() const {
		return new Iter(*this);
	}
};

struct Alt : public Rule {
	//Seq(const string& name) :
	//	Rule(name) { }
	Alt(const Alt&) = default;
	Alt(const string& name, const string& left, const string& right) :
		Rule(name, left, vector<string>{right}) { }
	Alt(const string& name, const string& left, const vector<string>& right) : Rule(name, left, right) { }
	Alt(const string& name, const string& left, const vector<Syntagma*>& right) : Rule(name, left, right) { }
	Alt(const vector<Syntagma*>& right) : Rule(right) { }
	virtual string show_def() const {
		string ret = "Sequental rule: " + left->name + " = ";
		for (auto s : right) ret += s->show_ref() + " ";
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
	virtual Syntagma* clone() const {
		return new Alt(*this);
	}
};

struct Opt : public Rule {
	//Seq(const string& name) :
	//	Rule(name) { }
	Opt(const Opt&) = default;
	Opt(const string& name, const string& left, const string& right) :
		Rule(name, left, vector<string>{right}) { }
	Opt(const string& name, const string& left, const vector<string>& right) : Rule(name, left, right) { }
	Opt(const string& name, const string& left, const vector<Syntagma*>& right) : Rule(name, left, right) { }
	Opt(const vector<Syntagma*>& right) : Rule(right) { }
	virtual string show_def() const {
		string ret = "Sequental rule: " + left->name + " = ";
		for (auto s : right) ret += s->show_ref() + " ";
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
	virtual Syntagma* clone() const {
		return new Opt(*this);
	}
};

} // namespace synt

typedef rule::Lexeme Lexeme;
typedef rule::Keyword Keyword;
typedef rule::Keywords Keywords;
typedef rule::Regexp Regexp;
typedef rule::Nonterm Nonterm;
typedef rule::Nonterms Nonterms;
typedef rule::Empty Empty;

inline Keyword* KW(const string& s) { return new Keyword(s); }
inline Nonterm* NT(const string& s) { return new Nonterm(s); }
inline Regexp*  RE(const string& s, const string& b) { return new Regexp(s, b); }

typedef rule::Rule Rule;

#define ACCESSOR(Synt) \
inline rule::Synt* Synt(const string& name, const string& left, const string& right) { \
	return new rule::Synt(name, left, right); \
} \
inline rule::Synt* Synt(const string& name, const string& left, const vector<string>& right) { \
	return new rule::Synt(name, left, right); \
} \
inline rule::Synt* Synt(const string& name, const string& left, const vector<Syntagma*>& right) { \
	return new rule::Synt(name, left, right); \
} \
inline rule::Synt* Synt(const vector<Syntagma*>& right) { \
	return new rule::Synt(right); \
}

ACCESSOR(Seq)
ACCESSOR(Iter)
ACCESSOR(Alt)
ACCESSOR(Opt)

inline bool is_lexeme(Syntagma* s) { return dynamic_cast<const rule::Lexeme*>(s); }
inline bool is_nonterm(Syntagma* s) { return dynamic_cast<const rule::Nonterm*>(s); }
inline bool is_rule(Syntagma* s) { return dynamic_cast<const rule::Rule*>(s); }

}
