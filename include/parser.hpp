#pragma once

#include "grammar.hpp"

namespace dynaparse {
namespace parser {

struct Node;

typedef vector<Node> Tree;
typedef string::const_iterator StrIter;

struct Node {
	bool   final;
	const Tree*  tree;
	string body;
	Tree   next;
	const Rule*  rule;

	bool operator == (const string& s) const { return body == s; }
	bool operator != (const string& s) const { return !operator == (s); }
	bool matches(StrIter ch, StrIter end) const {
		assert(!tree);
		StrIter x = body.begin();
		for (; x != body.end() && ch != end; ++x, ++ch) {
			if (*x != *ch) return false;
		}
		return ch != end || x == body.end();
	}
};

inline Node createNode(map<string, Tree>& trees, Symb& s) {
	Node n;
	n.body = s.body;
	n.rule = nullptr;
	n.tree = nullptr;
	s.term = true;
	if (trees.count(s.body)) {
		n.tree = &trees.at(s.body);
		s.term = false;
	}
	return n;
}

inline Node* add(map<string, Tree>& trees, Tree& tree, vector<Symb>& ex) {
	assert(ex.size());
	Tree* m = &tree;
	Node* n = nullptr;
	for (auto& x : ex) {
		bool new_symb = true;
		for (Node& p : *m) {
			if (p == x.body) {
				n = &p;
				m = &p.next;
				x.term = !p.tree;
				new_symb = false;
				break;
			}
		}
		if (new_symb) {
			if (m->size()) m->back().final = false;
			m->push_back(createNode(trees, x));
			n = &m->back();
			n->final = true;
			m = &n->next;
		}
	}
	return n;
}

typedef Tree::const_iterator MapIter;

enum class Action { RET, BREAK, CONT };

inline Action act(stack<MapIter>& n, stack<StrIter>& m, StrIter ch, StrIter end, Expr& t) {
	if (const Rule* r = n.top()->rule) {
		t.rule = r;
		return Action::RET;
	} else if (ch + 1 == end)
		return Action::BREAK;
	else {
		n.push(n.top()->next.begin());
		m.push(++ch);
	}
	return Action::CONT;
}

inline StrIter parse_LL(Expr& t, StrIter x, StrIter end, const Tree& tree, bool initial = false) {
	if (initial || !tree.size()) return StrIter();
	stack<MapIter> n;
	stack<StrIter> m;
	stack<MapIter> childnodes;
	n.push(tree.begin());
	m.push(x);
	while (!n.empty() && !m.empty()) {
		if (const Tree* deeper = n.top()->tree) {
			t.children.push_back(Expr());
			childnodes.push(n.top());
			Expr& child = t.children.back();
			auto ch = parse_LL(child, m.top(), end, *deeper, n.top() == tree.begin());
			if (ch != StrIter()) {
				switch (act(n, m, ch, end, t)) {
				case Action::RET  : return ch;
				case Action::BREAK: return StrIter();
				case Action::CONT : continue;
				}
			} else {
				t.children.pop_back();
				childnodes.pop();
			}
		} else if (n.top()->matches(m.top(), end)) {
			switch (act(n, m, m.top(), end, t)) {
			case Action::RET  : return m.top();
			case Action::BREAK: return StrIter();
			case Action::CONT : continue;
			}
		}
		while (n.top()->final) {
			n.pop();
			m.pop();
			if (!childnodes.empty() && childnodes.top() == n.top()) {
				t.children.pop_back();
				childnodes.pop();
			}
			if (n.empty() || m.empty()) return StrIter();
		}
		++n.top();
	}
	return StrIter();
}

} // parser namespace

class Parser {
public :
	Parser(Grammar& gr) : grammar(gr), trees() {
		for (Rule& rule : grammar.rules) {
			trees[rule.left];
		}
		for (Rule& rule : grammar.rules) {
			parser::Tree& tree = trees[rule.left];
			parser::Node* n = add(trees, tree, rule.right);
			n->rule = &rule;
		}
	}
	bool parse(const string& src, Expr& expr, const string& type);

private :
	Grammar& grammar;
	map<string, parser::Tree> trees;
};


bool Parser::parse(const string& src, Expr& expr, const string& type) {
	return parse_LL(expr, src.begin(), src.end(), trees[type]) + 1 == src.end();
}

}
