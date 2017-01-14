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

vector<string> show_vect(const Node& n);

vector<string> show_vect(const Tree& t) {
	vector<string> ret;
	for (const Node& n : t) {
		vector<string> v = show_vect(n);
		for (string& s : v) ret.push_back(s);
	}
	return ret;
}

vector<string> show_vect(const Node& n) {
	vector<string> ret;
	vector<string> next = show_vect(n.next);
	if (next.size()) {
		for (string& s : next) {
			ret.push_back(n.symb->name + (n.rule ? "[" + n.rule->show() + "]" : "") + " " + s);
		}
	} else {
		ret.push_back(n.symb->name + (n.rule ? "[" + n.rule->show() + "]" : ""));
	}
	return ret;
}

string show(const Node& n) {
	string ret;
	vector<string> vect = show_vect(n);
	for (string& s : vect) {
		ret += "\t" + s + "\n";
	}
	return ret;
}

string show(const Tree& tree) {
	string ret;
	for (const Node& n : tree) {
		ret += show(n);
	}
	return ret;
}

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

inline Node* add(map<string, Tree>& trees, Tree& tree, const vector<Syntagma*>& ex) {
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
	} /*else if (ch == end)
		return Action::BREAK;*/
	else {
		n.push(n.top()->next.begin());
		m.push(ch);
	}
	return Action::CONT;
}

inline Expr* parse_LL(StrIter& beg, StrIter end, Skipper* skipper, const Tree& tree, bool initial = false) {
	if (initial || !tree.size()) {
		return nullptr;
	}
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
		const Node& node = *n.top();

		//cout << "node: \n" << show(node) << endl;

		if (const Tree* deeper = node.tree) {
			//cout << "deeper: \n" << show(*deeper) << endl;
			childnodes.push(n.top());
			if (Expr* child = parse_LL(ch, end, skipper, *deeper, (n.top() == tree.begin()) && (deeper == deeper->begin()->tree))) {
				children.push_back(child);
				switch (act(n, m, beg, ch, end, rule)) {
				case Action::RET  : beg = ch; return new expr::Seq(b, ch, rule, children);
				case Action::BREAK: return nullptr;
				case Action::CONT : continue;
				}
			} else {
				childnodes.pop();
			}
		} else if (node.symb->matches(ch, end)) {
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
		for (Rule* rule : grammar.rules) {
			rule::Ref* nt = dynamic_cast<rule::Ref*>(rule->left);
			parser::Tree& tree = trees[nt->name];
			parser::Node* n = nullptr;
			if (rule::NaryOperator* op = dynamic_cast<rule::NaryOperator*>(rule->right)) {
				n = add(trees, tree, op->operands);
			} else {
				n = add(trees, tree, {rule->right});
			}
			n->rule = rule;
		}
	}
	Expr* parse(string& src, const string& type);

	Grammar& grammar;
	map<string, parser::Tree> trees;
};

string show(const Parser& parser) {
	string ret;
	for (auto& p : parser.trees) {
		if (p.second.size()) {
			ret += "tree for " + p.first + ":\n";
			ret += show(p.second) + "\n";
		}
	}
	return ret;
}


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
