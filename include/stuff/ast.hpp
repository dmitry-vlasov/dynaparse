#pragma once
#include "stuff/expr.hpp"

namespace dynaparse {

struct Type {
	~Type();
	uint ind;
	uint id;
	vector<Type*>     sup;
	map<Type*, Rule*> supers;
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
	Symbols term;
};

inline Type::~Type() {
	for (auto p : supers) delete p.second;
}

}
