#pragma once

#include "syntagma.hpp"
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

inline Node createNode(map<string, Tree>& trees, const Symb* s) {
	Node n;
	n.symb = s;
	n.rule = nullptr;
	n.tree = nullptr;
	if (const symb::Nonterm* nt = dynamic_cast<const symb::Nonterm*>(s)) {
		assert(nt && "must be non-terminal");
		assert(trees.count(nt->name) && "non-terminal is not declared");
		n.tree = &trees.at(nt->name);
	}
	return n;
}

inline Node* add(map<string, Tree>& trees, Tree& tree, vector<Syntagma*>& ex) {
	assert(ex.size());
	Tree* m = &tree;
	Node* n = nullptr;
	for (Syntagma* ss : ex) {
		rule::Ref* r = dynamic_cast<rule::Ref*>(ss);
		if (!r) {
			std::cerr << "syntagma " << ss->show() << " must be a symbol reference" <<std::endl;
			throw std::exception();
		}
		bool new_symb = true;
		for (Node& p : *m) {
			if (p.symb->equals(r->ref)) {
				n = &p;
				m = &p.next;
				new_symb = false;
				break;
			}
		}
		if (new_symb) {
			if (m->size()) m->back().final = false;
			m->push_back(createNode(trees, r->ref));
			n = &m->back();
			n->final = true;
			m = &n->next;
		}
	}
	return n;
}

typedef Tree::const_iterator MapIter;

enum class Action { RET, BREAK, CONT };

inline Action act(stack<MapIter>& n, stack<StrIter>& m, StrIter beg, StrIter ch, StrIter end, const Rule*& rule) {
	if (const Rule* r = n.top()->rule) {
		rule = r;
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

	vector<Expr*> children;
	const Rule* rule = nullptr;

	stack<MapIter> n;
	stack<StrIter> m;
	stack<MapIter> childnodes;
	n.push(tree.begin());
	m.push(beg);
	StrIter b = beg;
	StrIter ch = beg;
	while (!n.empty() && !m.empty()) {
		ch = m.top();
		skip(skipper, ch, end);
		StrIter c = ch;
		if (const Tree* deeper = n.top()->tree) {
			childnodes.push(n.top());
			if (Expr* child = parse_LL(ch, end, skipper, *deeper, n.top() == tree.begin())) {
				children.push_back(child);
				switch (act(n, m, beg, ch, end, rule)) {
				case Action::RET  : beg = ch; return new expr::Seq(b, ch, rule, children);
				case Action::BREAK: return nullptr;
				case Action::CONT : continue;
				}
			} else {
				childnodes.pop();
			}
		} else if (n.top()->symb->matches(ch, end)) {
			children.push_back(new expr::Lexeme(c, ch));
			switch (act(n, m, beg, ch, end, rule)) {
			case Action::RET  : beg = ch; return new expr::Seq(b, ch, rule, children);
			case Action::BREAK: return nullptr;
			case Action::CONT : continue;
			}
		}
		while (n.top()->final) {
			n.pop();
			m.pop();
			if (!childnodes.empty() && childnodes.top() == n.top()) {
				delete children.back();
				children.pop_back();
				childnodes.pop();
			}
			if (n.empty() || m.empty()) return nullptr;
		}
		++n.top();
	}
	return nullptr;
}

} // parser namespace

class Parser {
public :
	Parser(Grammar& gr) : grammar(gr), trees() {
		for (Symb* s : grammar.symbs) {
			if (symb::Nonterm* nt = dynamic_cast<symb::Nonterm*>(s)) {
				trees[nt->name];
			}
		}
		for (Rule& rule : grammar.rules) {
			rule::Ref* nt = dynamic_cast<rule::Ref*>(rule.left);
			rule::Operator* op = dynamic_cast<rule::Operator*>(rule.right);
			parser::Tree& tree = trees[nt->name];
			parser::Node* n = add(trees, tree, op->operands);
			n->rule = &rule;
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
