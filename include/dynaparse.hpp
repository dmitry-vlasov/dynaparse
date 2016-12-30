#pragma once
#include <string>
#include <map>
#include <vector>

namespace dynaparse {

using std::string;
using std::map;
using std::vector;

typedef std::uint32_t uint;

#define UNDEF_UINT 0xFFFFFFFF
#define UNDEF_LIT  0x0FFFFFFF

template<class T> struct Undef;
template<> struct Undef<uint> {
	static uint get()        { return UNDEF_UINT; }
	static bool is(uint x)   { return x == UNDEF_UINT; }
	static void set(uint& x) { x = UNDEF_UINT; }
};

template<class T> struct Undef<T*> {
	static T*   get()      { return nullptr; }
	static bool is(T* x)   { return x == nullptr; }
	static void set(T*& x) { x = nullptr;  }
};

struct Table {
	typedef std::map<string, uint> Table_;
	typedef std::vector<string> Strings_;

	const Table& get() { return mod(); }
	Table& mod() { Table table; return table; }

	uint getInt(const string& str) const {
		if (table.find(str) == table.end())
			return -1;
		else
			return table.find(str)->second;
	}
	uint toInt(const string& str) {
		if (table.find(str) == table.end()) {
			int ind = table.size();
			table[str] = ind;
			strings.push_back(str);
		}
		return table[str];
	}
	const string& toStr (uint i) const {
		if (i >= strings.size()) {
			static string str = "<UNDEF>";
			return str;
		}
		return strings[i];
	}
	Strings_ strings;
	Table_   table;

private:
	Table() : strings(), table() { }
};

class indent {
	int  num;
	char del;
public:
	indent(int n = 1, char d = '\t') : num(n), del(d) {
	}
	void write(ostream& os) {
		while (num --) os << del;
	}
	static string paragraph(const string& str, string d = "\t") {
		string indented;
		for (char ch : str) {
			if (ch == '\n') indented += "\n" + d;
			else            indented += ch;
		}
		return indented;
	}
};


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

struct Rule;

void parse_expr(Expr& ex);
void parse_term(Expr& ex, Rule* rule);

struct Rules {
	struct Node;
	typedef vector<Node> Map;
	Rule*& add(const Expr& ex);
	Map map;
};

struct Rules::Node {
	Node(Symbol s) : symb(s), tree(), level(), rule(nullptr) { }
	Symbol symb;
	Rules  tree;
	uint   level;
	Rule*  rule;
};


struct Type {
	~Type();
	uint ind;
	uint id;
	vector<Type*>     sup;
	map<Type*, Type*> supers;
	Rules             rules;
};

struct Vars {
	vector<Symbol> v;
};

struct Rule {
	uint  ind;
	uint  id;
	Type* type;
	Vars  vars;
	Expr  term;
};

inline Type::~Type() {
	for (auto p : supers) delete p.second;
}

}
