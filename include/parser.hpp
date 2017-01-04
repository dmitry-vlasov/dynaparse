#pragma once

#include "grammar.hpp"

namespace dynaparse {
namespace xxx {

struct Tree {
	struct Node;
	typedef vector<Node> Map;
	Map map;
};

struct ExtType;

typedef string::const_iterator StrIter;

struct Tree::Node {
	bool     is_fin;
	ExtType* type;
	string   body;
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

struct ExtType {
	Type* type;
	Tree tree;
	map<ExtType*, Rule*> supers;
};

class Parser {
public :
	Parser(Grammar& gr) : grammar(gr), types() {
		for (Type& type : grammar.types) {
			types[type.name].type = &type;
		}
		for (Rule& rule : grammar.rules) {
			ExtType& type = types[rule.left];
			Tree::Node* n = add(type.tree, rule.right);
			n->rule = &rule;
		}
		std::cout << gr.show() << std::endl;
	}
	void parse(const string& src, Expr& expr, const string& type);

private :
	Tree::Node createNode(Symb& s);
	Tree::Node* add(Tree& tree, vector<Symb>& ex);
	Grammar& grammar;
	map<string, ExtType> types;
};

inline Tree::Node Parser::createNode(Symb& s){
	Tree::Node n;
	n.body = s.body;
	n.rule = nullptr;
	n.type = nullptr;
	s.term = true;
	if (types.count(s.body)) {
		n.type = &types[s.body];
		s.term = false;
	}
	return n;
}

inline Tree::Node* Parser::add(Tree& tree, vector<Symb>& ex) {
	assert(ex.size());
	Tree* m = &tree;
	Tree::Node* n = nullptr;
	for (auto& x : ex) {
		bool new_symb = true;
		for (Tree::Node& p : m->map) {
			if (p == x.body) {
				n = &p;
				m = &p.tree;
				x.term = !p.type;
				new_symb = false;
				break;
			}
		}
		if (new_symb) {
			if (m->map.size()) m->map.back().is_fin = false;
			m->map.push_back(createNode(x));
			n = &m->map.back();
			n->is_fin = true;
			m = &n->tree;
		}
	}
	return n;
}

typedef Tree::Map::const_iterator MapIter;

inline Rule* find_super(ExtType* type, ExtType* super) {
	auto it =type->supers.find(super);
	if (it != type->supers.end())
		return it->second;
	else
		return nullptr;
}

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
		n.push(n.top()->tree.map.begin());
		m.push(++ch);
	}
	return Action::CONT;
}

inline StrIter parse_LL(Expr& t, StrIter x, StrIter end, ExtType* type, uint ind, bool initial = false) {
	if (!initial && type->tree.map.size()) {
		//t.kind = Expr::NODE;

		stack<MapIter> n;
		stack<StrIter> m;
		stack<MapIter> childnodes;
		n.push(type->tree.map.begin());
		m.push(x);
		while (!n.empty() && !m.empty()) {
			if (ExtType* tp = n.top()->type) {
				t.children.push_back(Expr());
				childnodes.push(n.top());
				Expr& child = t.children.back();
				auto ch = parse_LL(child, m.top(), end, tp, ind, n.top() == type->tree.map.begin());
				if (ch != StrIter()) {
					switch (act(n, m, ch, end, t, ind)) {
					case Action::RET  : return ch;
					case Action::BREAK: goto out;
					case Action::CONT : continue;
					}
				} else {
					t.children.pop_back();
					childnodes.pop();
				}
			} else if (n.top()->matches(m.top(), end)) {
				switch (act(n, m, m.top(), end, t, ind)) {
				case Action::RET  : return m.top();
				case Action::BREAK: goto out;
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
				if (n.empty() || m.empty()) goto out;
			}
			++n.top();
		}
		out: ;
	}
	/*if (x->type) {
		if (x->type == type) {
			t = Expr(*x);
			return x;
		} else if (Rule* super = find_super(x->type, type)) {
			t = Expr(super);
			t.children.push_back(Expr(*x));
			return x;
		}
	}*/
	return StrIter();
}

inline void Parser::parse(const string& src, Expr& expr, const string& type) {
	StrIter it = parse_LL(expr, src.begin(), src.end(), &types[type], 0);
	if (it + 1 != src.end()) {
		std::cout << "FUCK" << std::endl;
	}
}


}}
