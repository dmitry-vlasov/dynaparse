#pragma once
#include "table.hpp"

namespace dynaparse {

struct Literal {
	Literal(): lit(UNDEF_LIT), var(false), end(false), rep(false), fin(false) { }
	Literal(uint l, bool v = false) : lit (l), var (v), end(false), rep(false), fin(false) { }
	Literal(const string& s) : lit(Table::mod().toInt(s)), var(false), end(false), rep(false), fin(false) { }

	bool operator == (const Literal& s) const {
		return lit == s.lit && var == s.var;
	}
	bool operator != (const Literal& s) const {
		return !operator ==(s);
	}
	bool operator < (const Literal& s) const {
		return lit < s.lit;
	}
	bool is_undef() const { return lit == UNDEF_LIT; }
	static bool is_undef(uint lit) { return lit == UNDEF_LIT; }
	uint lit:28;

	// Flags
	bool var:1; //< is variable
	bool end:1; //< is end of an expression
	bool rep:1; //< is replaceable var
	bool fin:1; //< final node in a tree (in a horizontal iteration)
};


struct Type;

struct Symbol : public Literal {
	Symbol(const string& s, Type* t = nullptr) : Literal(s), type(t) { }
	Symbol() : Literal(), type(nullptr) { }
	Symbol(uint l): Literal(l), type(nullptr) { }
	Symbol(const Literal s, bool v = false) :
	Literal(s.lit, v), type(nullptr) { }
	Symbol(const Literal s, Type* tp, bool v = false) :
	Literal(s.lit, v), type(tp) { }

	bool operator == (const Symbol& s) const {
		return Literal::operator == (s) && type == s.type;
	}
	bool operator != (const Symbol& s) const {
		return !operator ==(s);
	}
	bool operator < (const Symbol& s) const {
		return
			type == s.type ?
			Literal::operator < (s.lit) :
			type < s.type;
	}
	Type* type;
	struct Hash {
		typedef size_t result_type;
		typedef Symbol argument_type;
		size_t operator() (Symbol s) const {
			return hash(s.lit);
		}
	private:
		static std::hash<uint> hash;
	};
};

}
