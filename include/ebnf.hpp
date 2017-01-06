#pragma once

#include "grammar.hpp"

namespace dynaparse {

Rule::Rule(const string& left, const string& right) :
	left_str(left), right_str(), left(nullptr), right(), is_leaf(true), semantic(nullptr) {
	std::stringstream ss(right);
    std::string item;
    while (std::getline(ss, item, ' ')) {
        right_str.push_back(item);
    }
}

Grammar& Grammar::operator << (const Rule& r) {
	rules.push_back(new Rule(r));

	/*while (Rule* x = extract(r)) {
		q.push(r);
		q.push(x);
	}*/


	Rule& rule = *rules.back();
	if (!nonterm_map.count(rule.left_str)) {
		std::cerr << "undefined non-terminal: " << rule.left_str << std::endl;
		throw std::exception();
	}
	rule.left = nonterm_map[rule.left_str];
	for (string& s : rule.right_str) {
		if (nonterm_map.count(s)) {
			rule.right.push_back(nonterm_map[s]);
			rule.is_leaf = false;
			continue;
		}
		if (keyword_map.count(s)) {
			rule.right.push_back(keyword_map[s]);
			continue;
		}
		if (regexp_map.count(s)) {
			rule.right.push_back(regexp_map[s]);
			continue;
		}
		std::cerr << "undefined symbol: " << s << std::endl;
		throw std::exception();
	}
	return *this;
}

string Grammar::new_non_term() {
	while (true) {
		string nn = "N_" + std::to_string(c++);
		if (!nonterm_map.count(nn)) {
			*this << Nonterm(nn);
			return nn;
		}
	}
	return "";
}

Rule* Grammar::extract(Rule* r) {
	vector<string>& v = r->right_str;
	int d = 0;
	auto beg = v.end();
	auto end = v.end();
	for (auto it = v.begin(); it != v.end(); ++ it) {
		if (*it == "_{") {
			if (d == 0) beg = it;
			++ d;
		}
		if (*it == "_}") {
			--d;
			if (d == 0) end = it;
		}
	}
	if (beg != v.end() && end != v.end()) {
		vector<string> w(beg + 1, end);
		string nnt = new_non_term();
		v.insert(end + 1, nnt);
		v.erase(beg, end + 1);
		*this << Rule(nnt, w);
		return rules.back();
	} else {
		return nullptr;
	}
}


void Grammar::parse_EBNF() {
	//typedef vector<string>::const_iterator VectIter;
	queue<Rule*> q;
	for (Rule* r : rules) q.push(r);
	while (!q.empty()) {
		Rule* r = q.front(); q.pop();
		if (Rule* x = extract(r)) {
			q.push(r);
			q.push(x);
		}
	}

}

}
