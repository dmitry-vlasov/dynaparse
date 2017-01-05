#pragma once

#include "grammar.hpp"

namespace dynaparse {
namespace parser {

struct Node;

typedef vector<Node> Tree;

struct Node {
	bool   final;
	Symb*  symb;
	Tree   next;
	const Tree* tree;
	const Rule* rule;

	bool operator == (const string& s) const { return symb->body == s; }
	bool operator != (const string& s) const { return !operator == (s); }
};

inline Node createNode(map<string, Tree>& trees, Skipper* skipper, Symb& s) {
	Node n;
	n.symb = &s;
	n.rule = nullptr;
	n.tree = nullptr;
	s.term = true;
	if (trees.count(s.body)) {
		n.tree = &trees.at(s.body);
		s.term = false;
	}
	return n;
}

inline Node* add(map<string, Tree>& trees, Skipper* skipper, Tree& tree, vector<Symb>& ex) {
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
			m->push_back(createNode(trees, skipper, x));
			n = &m->back();
			n->final = true;
			m = &n->next;
		}
	}
	return n;
}

typedef Tree::const_iterator MapIter;

enum class Action { RET, BREAK, CONT };

inline Action act(stack<MapIter>& n, stack<StrIter>& m, StrIter beg, StrIter ch, StrIter end, Skipper* skipper, Expr* t) {
	if (const Rule* r = n.top()->rule) {
		t->rule = r;
		if (r->semantic) r->semantic(t, beg, ch);
		return Action::RET;
	} else if (ch + 1 == end)
		return Action::BREAK;
	else {
		n.push(n.top()->next.begin());
		while (ch != end && skipper(*ch)) ++ch;
		m.push(++ch);
	}
	return Action::CONT;
}

inline Expr* parse_LL(StrIter& beg, StrIter end, Skipper* skipper, const Tree& tree, bool initial = false) {
	if (initial || !tree.size()) return nullptr;
	Expr* t = new Expr();
	stack<MapIter> n;
	stack<StrIter> m;
	stack<MapIter> childnodes;
	n.push(tree.begin());
	m.push(beg);
	while (!n.empty() && !m.empty()) {
		if (const Tree* deeper = n.top()->tree) {
			childnodes.push(n.top());
			StrIter ch = m.top();
			if (Expr* child = parse_LL(ch, end, skipper, *deeper, n.top() == tree.begin())) {
				t->nodes.push_back(child);
				switch (act(n, m, beg, ch, end, skipper, t)) {
				case Action::RET  : beg = ch; return t;
				case Action::BREAK: goto out;
				case Action::CONT : continue;
				}
			} else {
				childnodes.pop();
			}
		} else if (n.top()->symb->matches(skipper, m.top(), end)) {
			switch (act(n, m, beg, m.top(), end, skipper, t)) {
			case Action::RET  : beg = m.top(); return t;
			case Action::BREAK: goto out;
			case Action::CONT : continue;
			}
		}
		while (n.top()->final) {
			n.pop();
			m.pop();
			if (!childnodes.empty() && childnodes.top() == n.top()) {
				delete t->nodes.back();
				t->nodes.pop_back();
				childnodes.pop();
			}
			if (n.empty() || m.empty()) goto out;
		}
		++n.top();
	}
	out :
	delete t;
	return nullptr;
}

} // parser namespace

class Parser {
public :
	Parser(Grammar gr) : grammar(gr), trees() {
		for (Rule& rule : grammar.rules) {
			trees[rule.left];
		}
		for (Rule& rule : grammar.rules) {
			parser::Tree& tree = trees[rule.left];
			parser::Node* n = add(trees, gr.skipper, tree, rule.right);
			n->rule = &rule;
		}
	}
	Expr* parse(const string& src, const string& type);

	const Grammar& getGrammar() const { return grammar; }
private :
	Grammar grammar;
	map<string, parser::Tree> trees;
};

Expr* Parser::parse(const string& src, const string& type) {
	StrIter beg = src.begin();
	if (Expr* expr = parse_LL(beg, src.end(), grammar.skipper, trees[type])) {
		++beg;
		while (beg != src.end() && grammar.skipper(*beg)) ++beg;
		if (beg == src.end()) return expr;
		delete expr;
	}
	return nullptr;
}

}
