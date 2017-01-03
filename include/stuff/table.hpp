#pragma once
#include "std.hpp"

namespace dynaparse {

struct Table {
	typedef std::map<string, uint> Table_;
	typedef std::vector<string> Strings_;

	static const Table& get() { return mod(); }
	static Table& mod() { static Table table; return table; }

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
	int      ind;

private:
	Table() : strings(), table(), ind(0) { }
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

}
