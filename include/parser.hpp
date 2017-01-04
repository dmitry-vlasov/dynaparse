#pragma once

#include "grammar.hpp"

namespace dynaparse {
namespace xxx {

struct Node;

typedef vector<Node> Tree;
typedef string::const_iterator StrIter;

struct Node {
	bool   is_fin;
	Tree*  type;
	string body;
	bool operator == (const string& s) const { return body == s; }
	bool operator != (const string& s) const { return !operator == (s); }
	bool matches(StrIter ch, StrIter end) const {
		assert(!type);
		StrIter x = body.begin();
		for (; x != body.end() && ch != end; ++x, ++ch) {
			if (*x != *ch) return false;
		}
		return ch != end || x == body.end();
	}
	Tree  tree;
	uint  level;
	Rule* rule;
};

class Parser {
public :
	Parser(Grammar& gr) : grammar(gr), trees() {
		for (Rule& rule : grammar.rules) {
			trees[rule.left];
		}
		for (Rule& rule : grammar.rules) {
			Tree& tree = trees[rule.left];
			Node* n = add(tree, rule.right);
			n->rule = &rule;
		}
		std::cout << gr.show() << std::endl;
	}
	void parse(const string& src, Expr& expr, const string& type);

private :
	Node createNode(Symb& s);
	Node* add(Tree& tree, vector<Symb>& ex);
	Grammar& grammar;
	map<string, Tree> trees;
};

inline Node Parser::createNode(Symb& s){
	Node n;
	n.body = s.body;
	n.rule = nullptr;
	n.type = nullptr;
	s.term = true;
	if (trees.count(s.body)) {
		n.type = &trees[s.body];
		s.term = false;
	}
	return n;
}

inline Node* Parser::add(Tree& tree, vector<Symb>& ex) {
	assert(ex.size());
	Tree* m = &tree;
	Node* n = nullptr;
	for (auto& x : ex) {
		bool new_symb = true;
		for (Node& p : *m) {
			if (p == x.body) {
				n = &p;
				m = &p.tree;
				x.term = !p.type;
				new_symb = false;
				break;
			}
		}
		if (new_symb) {
			if (m->size()) m->back().is_fin = false;
			m->push_back(createNode(x));
			n = &m->back();
			n->is_fin = true;
			m = &n->tree;
		}
	}
	return n;
}

typedef Tree::const_iterator MapIter;

enum class Action { RET, BREAK, CONT };

inline Action act(stack<MapIter>& n, stack<StrIter>& m, StrIter ch, StrIter end, Expr& t, uint ind) {
	if (Rule* r = n.top()->rule) {
		//if (r->ind <= ind) {
			t.rule = r;
			return Action::RET;
		//} else
		//	return Action::BREAK;
	} else if (ch + 1 == end)
		return Action::BREAK;
	else {
		n.push(n.top()->tree.begin());
		m.push(++ch);
	}
	return Action::CONT;
}

inline StrIter parse_LL(Expr& t, StrIter x, StrIter end, Tree* type, uint ind, bool initial = false) {
	if (initial || !type->size()) return StrIter();
	stack<MapIter> n;
	stack<StrIter> m;
	stack<MapIter> childnodes;
	n.push(type->begin());
	m.push(x);
	while (!n.empty() && !m.empty()) {
		if (Tree* tp = n.top()->type) {
			t.children.push_back(Expr());
			childnodes.push(n.top());
			Expr& child = t.children.back();
			auto ch = parse_LL(child, m.top(), end, tp, ind, n.top() == type->begin());
			if (ch != StrIter()) {
				switch (act(n, m, ch, end, t, ind)) {
				case Action::RET  : return ch;
				case Action::BREAK: return StrIter();
				case Action::CONT : continue;
				}
			} else {
				t.children.pop_back();
				childnodes.pop();
			}
		} else if (n.top()->matches(m.top(), end)) {
			switch (act(n, m, m.top(), end, t, ind)) {
			case Action::RET  : return m.top();
			case Action::BREAK: return StrIter();
			case Action::CONT : continue;
			}
		}
		while (n.top()->is_fin) {
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

inline void Parser::parse(const string& src, Expr& expr, const string& type) {
	StrIter it = parse_LL(expr, src.begin(), src.end(), &trees[type], 0);
	if (it + 1 != src.end()) {
		std::cout << "FUCK" << std::endl;
	}
}

}}
