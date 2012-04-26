#include "bnode.hpp"
#include "bdd_op.hpp"


/* Not operator */
bnode operator!(const bnode& node) {
	return bnode(node.index, node.low, node.high);
}

/* Equality operator */
bool operator==(const bnode &lhs, const bnode &rhs) {
	return (lhs.index == rhs.index) && 
		   (lhs.high  == rhs.high)  && 
		   (lhs.low   == rhs.low);
}

/* If-then-else operator */
const bnode& ite(const bnode& test, const bnode& high, const bnode& low) {
	if (bddtrue == test)
		return high;
	if (test == bddfalse)
		return low;
	if (high == low)
		return high;
	return test;
}

/* Insertion operator */
std::ostream& operator<<(std::ostream& out, const bnode& node) {
	out << "(" << node.index;
	if (node.high) 
		out << " " << *(node.high);
	if (node.low) 
		out << " " << *(node.low);
	out << ")";
	return out;
}

bnode::bnode() {
	this->index = 0;
	this->high = 0;
	this->low = 0;
}

/* Parameterized constructor */
bnode::bnode(int i, bnode* h, bnode* l) {
	this->index = i;
	this->high  = h;
	this->low   = l;
}

/* Copy constructor */
bnode& bnode::operator=(const bnode &that) {
	if (this != &that) {
		this->index = that.index;
		this->high  = that.high;
		this->low   = that.low;
	}
	return *this;
}

/* Provide a mapping from bdd_op to the if-then-else form */
const bnode& bnode::apply(const bnode &that, const bdd_op op) {
	switch (op) {
		case (AND):		return ite(*this, that, bddfalse);
		case (XOR):		return ite(*this, !that, bddfalse);
		case (OR):		return ite(*this, bddtrue, that);
		case (NAND):	return ite(*this, !that, bddtrue);
		case (NOR):		return ite(*this, bddfalse, !that);
		case (IMP):		return ite(*this, that, bddtrue);
		case (BIIMP):	return ite(ite(*this, that, bddtrue), 
								   ite(*this, bddtrue, !that),
								   bddfalse);
		case (INVIMP):	return ite(*this, bddtrue, !that);
		default:
			throw;
	}
}
