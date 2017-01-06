#pragma once
#include <string>
#include <map>
#include <set>
#include <vector>
#include <stack>
#include <queue>

#include <utility>
#include <iostream>
#include <unistd.h>
#include <stdint.h>
#include <algorithm>
#include <cassert>
#include <regex>

namespace dynaparse {

using std::string;
using std::map;
using std::vector;
using std::ostream;
using std::pair;
using std::stack;
using std::set;
using std::regex;
using std::queue;

typedef uint32_t uint;

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

}
