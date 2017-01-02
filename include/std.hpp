#pragma once
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <stdint.h>
#include <algorithm>

namespace dynaparse {

using std::string;
using std::map;
using std::vector;
using std::ostream;

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
