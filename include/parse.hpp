#include "ast.hpp"
#include "expr.hpp"

namespace dynaparse {

typedef Rules::Map::const_iterator MapIter;

inline Rule* find_super(Type* type, Type* super) {
	auto it =type->supers.find(super);
	if (it != type->supers.end())
		return it->second;
	else
		return nullptr;
}

enum class Action { RET, BREAK, CONT };

inline Action act(stack<MapIter>& n, stack<Symbols::iterator>& m, Symbols::iterator ch, Expr& t, uint ind) {
	if (Rule* r = n.top()->rule) {
		if (r->ind <= ind) {
			t.val.rule = r;
			return Action::RET;
		} else
			return Action::BREAK;
	} else if (ch->end)
		return Action::BREAK;
	else {
		n.push(n.top()->tree.map.begin());
		m.push(++ch);
	}
	return Action::CONT;
}

Symbols::iterator parse_LL(Expr& t, Symbols::iterator x, Type* type, uint ind, bool initial = false) {
	if (!initial && type->rules.map.size()) {
		t.kind = Expr::NODE;


		stack<MapIter> n;
		stack<Symbols::iterator> m;
		stack<MapIter> childnodes;
		n.push(type->rules.map.begin());
		m.push(x);
		while (!n.empty() && !m.empty()) {
			if (Type* tp = n.top()->symb.type) {
				t.children.push_back(Expr());
				childnodes.push(n.top());
				Expr& child = t.children.back();
				auto ch = parse_LL(child, m.top(), tp, ind, n.top() == type->rules.map.begin());
				if (ch != Symbols::iterator()) {
					switch (act(n, m, ch, t, ind)) {
					case Action::RET  : return ch;
					case Action::BREAK: goto out;
					case Action::CONT : continue;
					}
				} else {
					t.children.pop_back();
					childnodes.pop();
				}
			} else if (n.top()->symb == *m.top()) {
				switch (act(n, m, m.top(), t, ind)) {
				case Action::RET  : return m.top();
				case Action::BREAK: goto out;
				case Action::CONT : continue;
				}
			}
			while (n.top()->symb.fin) {
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
	if (x->type) {
		if (x->type == type) {
			t = Expr(*x);
			return x;
		} else if (Rule* super = find_super(x->type, type)) {
			t = Expr(super);
			t.children.push_back(Expr(*x));
			return x;
		}
	}
	return Symbols::iterator();
}

}
