#ifndef YABDD_BDD_H
#define YABDD_BDD_H

#include "bnode.hpp"
#include "bdd_op.hpp"

class bdd {
public:
	bdd(const bdd& that);
	bdd(const bnode& root_node);

	const bdd& apply(const bdd& that, bdd_op op);
	const bdd& operator&=(const bdd& that) { return this->apply(that, AND); }

	friend std::ostream& operator<< (std::ostream& out, const bdd& btree);

private:
	bnode root;
};


bdd::bdd(const bdd& that) {
	this->root = that.root;
}

bdd::bdd(const bnode& root_node) {
	this->root = root_node;
}

const bdd& bdd::apply(const bdd& that, const bdd_op op) {
	return (this->root).apply(that.root, op);
}

std::ostream& operator<< (std::ostream& out, const bdd& btree) {
	return out << btree.root;
}

#endif
