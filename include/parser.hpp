#pragma once

#include "ebnf.hpp"
#include "expr.hpp"

namespace dynaparse {
namespace parser {

struct Node;

typedef vector<Node> Tree;

struct Node {
	bool   final;
	Tree   next;
	const Symb* symb;
	const Tree* tree;
	const Rule* rule;
};

inline Node createNode(map<string, Tree>& trees, Skipper* skipper, const Symb* s) {
	Node n;
	n.symb = s;
	n.rule = nullptr;
	n.tree = nullptr;
	if (!s->lexeme()) {
		const Nonterm* nt = dynamic_cast<const Nonterm*>(s);
		assert(nt && "must be non-terminal");
		assert(trees.count(nt->name) && "non-terminal is not declared");
		n.tree = &trees.at(nt->name);
	}
	return n;
}

inline Node* add(map<string, Tree>& trees, Skipper* skipper, Tree& tree, vector<Symb*>& ex) {
	assert(ex.size());
	Tree* m = &tree;
	Node* n = nullptr;
	for (Symb* s : ex) {
		bool new_symb = true;
		for (Node& p : *m) {
			if (p.symb->equals(s)) {
				n = &p;
				m = &p.next;
				new_symb = false;
				break;
			}
		}
		if (new_symb) {
			if (m->size()) m->back().final = false;
			m->push_back(createNode(trees, skipper, s));
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
		t->beg = beg;
		t->end = ch;
		//if (r->semantic) r->semantic(t);
		return Action::RET;
	} else if (ch == end)
		return Action::BREAK;
	else {
		n.push(n.top()->next.begin());
		m.push(ch);
	}
	return Action::CONT;
}

inline Expr* parse_LL(StrIter& beg, StrIter end, Skipper* skipper, const Tree& tree, bool initial = false) {
	if (initial || !tree.size()) return nullptr;
	skip(skipper, beg, end);
	Expr* t = new Expr();
	stack<MapIter> n;
	stack<StrIter> m;
	stack<MapIter> childnodes;
	n.push(tree.begin());
	m.push(beg);
	while (!n.empty() && !m.empty()) {
		StrIter ch = m.top();
		if (const Tree* deeper = n.top()->tree) {
			childnodes.push(n.top());
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
		} else if (n.top()->symb->matches(skipper, ch, end)) {
			switch (act(n, m, beg, ch, end, skipper, t)) {
			case Action::RET  : beg = ch; return t;
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
	Parser(Grammar& gr) : grammar(gr), trees() {
		for (Rule* rule : grammar.rules) {
			trees[rule->left->name];
		}
		for (Rule* rule : grammar.rules) {
			parser::Tree& tree = trees[rule->left->name];
			parser::Node* n = add(trees, gr.skipper, tree, rule->right);
			n->rule = rule;
		}
	}
	Expr* parse(string& src, const string& type);

	const Grammar& getGrammar() const { return grammar; }
private :
	Grammar& grammar;
	map<string, parser::Tree> trees;
};

Expr* Parser::parse(string& src, const string& type) {
	StrIter beg = src.begin();
	if (Expr* expr = parse_LL(beg, src.end(), grammar.skipper, trees[type])) {
		while (beg != src.end() && grammar.skipper(*beg)) ++beg;
		if (beg == src.end()) return expr;
		delete expr;
	}
	return nullptr;
}

}
