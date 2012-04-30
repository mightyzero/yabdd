#ifndef YABDD_BDD_H
#define YABDD_BDD_H

#include <iostream>
#include <cstdio>
#include "assert.h"

#define PAIR(a,b)      ((unsigned int)((((unsigned int)a)+((unsigned int)b))*(((unsigned int)a)+((unsigned int)b)+((unsigned int)1))/((unsigned int)2)+((unsigned int)a)))
#define TRIPLE(a,b,c)  ((unsigned int)(PAIR((unsigned int)c,PAIR(a,b))))
#define NODEHASH(lvl, l, h)		(TRIPLE((lvl), (l), (h)) % bddnodesize);

int bddnodesize = 0;
int varsetsize = 0;
bnode* NodeTable[];		/* T: u -> (i, h, l) */

struct bnode {
public:
	int index; // lower index = closer to the top
	int high;  // pointer to the THEN node
	int low; // pointer to the ELSE node

public:
	// CONSTRUCTORS
	bnode() {
		this->index = 0;
		this->high = 0;
		this->low = 0;
	}

	bnode(int i) {
		this->index = i;
		this->high = 1;
		this->low  = 0;
	}

	bnode(int i, int h, int l) {
		this->index = i;
		this->high  = h;
		this->low   = l;
	}

	bnode& operator=(const bnode &that) {
		if (this != &that) {
			this->index = that.index;
			this->high  = that.high;
			this->low   = that.low;
		}
		return *this;
	}

	bool operator==(const bnode& that) const  {
		return (this->index == that.index && this->high == that.high && this->low == that.low);
	}
	bool operator<(const bnode& that) const {
		return (((that.index < 2) && (this->index > 1)) ||
				((this->index > 1) && (this->index <= that.index)));
	}

	bool operator<=(const bnode& that) const {
		return ((that.index < 2) ||
				((this->index > 1) && (this->index <= that.index)));
	}

	const bnode& apply(const bnode& that, const bdd_op op); // Map op to ITE form

private:
	friend std::ostream& operator<<(std::ostream& out, const bnode& node);
};

const bnode bnode_0(0, 0, 0); // 0-sink
const bnode bnode_1(1, 0, 0); // 1-sink

const bnode operator !(const bnode& node) {
	return bnode(node.index, node.low, node.high);
}

const bnode& bnode_min(const bnode& a, const bnode& b) {
	return ((a < b) ? a : b);
}

const bnode& bnode_min(const bnode& a, const bnode& b, const bnode& c)  {
	return bnode_min(bnode_min(a, b), bnode_min(a, c));
}

/* Insertion operator */
std::ostream& operator<<(std::ostream& out, const bnode& node) {
	if (node.index > 1)	{ out << "("; }
	out << node.index;
	if (node.high)		{ out << " " << *(node.high); }
	if (node.low)		{ out << " " << *(node.low);  }
	if (node.index > 1)	{ out << ")"; }
	return out;
}

class bdd {
private:
	bnode root;

public:
	// Constructors
	bdd();
	bdd(const bdd& that);
	bdd(const bnode& root_node);

	// Boolean Operators
	const bdd operator&(const bdd& that) const;
	const bdd operator|(const bdd& that) const;
	const bdd operator>>(const bdd& that) const;

	// Boolean assignment operators
	const bdd& operator&=(const bdd& that);
	const bdd& operator|=(const bdd& that);
	const bdd& operator>>=(const bdd& that);

	// Comparison operators
	bool operator==(const bdd& that) const;

private:
	friend int bdd_init(int initnodesize, int cachesize);
	friend int bdd_setnumvar(int num);

	friend bdd bdd_make(const bnode& node, const bdd& high, const bdd& low);

	friend int bdd_var(const bdd& btree);
	friend const bdd bdd_high(const bdd& btree);
	friend const bdd bdd_low(const bdd& btree);

	friend bnode& bdd_ithvar(int i);

	friend const bdd
			bdd_restrict(const bdd& btree, const bnode& node, bool val);
	friend const bdd bdd_ite(const bdd& I, const bdd& T, const bdd& E);

	friend const bdd bdd_and(const bdd& input1, const bdd& input2);
	friend const bdd bdd_or(const bdd& input1, const bdd& input2);
	friend const bdd bdd_impl(const bdd& input1, const bdd& input2);

	friend bdd satone(const bdd& btree);
	friend double satcount(const bdd& btree);

	friend std::ostream& operator<<(std::ostream& out, const bdd& btree);
};

// bdd constants
const bdd bddtrue(bnode_1);
const bdd bddfalse(bnode_0);

// START OF FRIEND FUNCTIONS ///////////////////////////////////////////////////
int bdd_init(int initnodesize, int cachesize) {
	bddnodesize = initnodesize;
	NodeTable = new bnode*[initnodesize];
	NodeTable[0] = &bnode_0;
	NodeTable[1] = &bnode_1;
	return 0;
}

int bdd_setnumvar(int num) {
	varsetsize = num;
	return 0;
}

bnode& bdd_makenode(int var, const bnode* high, const bnode* low) {
	unsigned int hash = NODEHASH(var, high, low);
	if (NodeTable[hash] == 0) {
		bnode newNode = new bnode(var, high, low);
		NodeTable[hash] = &newNode;
	}
	return NodeTable[hash];
}


// TODO: reference a map to make sure the bdd returned is unique.
// TODO: use a separate bnode_make function.
// TODO: reference a map to make sure the bnode used is unique.
// TODO: after that, return a reference instead of a new object

int bdd_var(const bdd& btree) {
	return btree.root.index;
}

const bdd bdd_high(const bdd& btree) {
	if (btree.root.high)
		return bdd(*(btree.root.high));
}
const bdd bdd_low(const bdd& btree) {
	if (btree.root.low)
		return bdd(*(btree.root.low));
}

bdd bdd_ithvar(int i) {
	return bdd(bnode(i + 2));
}

/** Shannon co-factor expansion wrt variable v (represented by bnode node)
 node index should always be at most btree root index */
const bdd bdd_restrict(const bdd& btree, const bnode& node, bool val) {
	printf("%i %i\n", btree.root.index, node.index);
	if (btree == bddtrue) {
		printf("is bddtrue\n");
		return bddtrue;
	}
	if (btree == bddfalse) {
		printf("is bddfalse");
		return bddfalse;
	}
	if (btree.root < node) {
		return btree;
	} else if (node < btree.root) {
		return bdd_make(node, bdd_restrict(bdd_high(btree), node, val),
				bdd_restrict(bdd_low(btree), node, val));
	} else { /* (btree.root.index == node.index) */
		if (val) {
			return bdd_restrict(bdd_high(btree), node, val);
		} else {
			return bdd_restrict(bdd_low(btree), node, val);
		}
	}
}

// Non-member If-then-else BDD operator
const bdd bdd_ite(const bdd& I, const bdd& T, const bdd& E) {
	// Base cases
	if (I == bddtrue) return T;
	if (I == bddfalse) return E;
	if (T == E) return T;
	if (T == bddtrue && E == bddfalse) return I;

	// General cases
	// splitting variable, must be the topmost one among the root
	bnode r = bnode_min(I.root, T.root, E.root);
//	printf("%i %i %i %i\n", I.root.index, T.root.index, E.root.index, r.index);
	bdd Ixt = bdd_restrict(I, r, true);
	bdd Txt = bdd_restrict(T, r, true);
	bdd Ext = bdd_restrict(E, r, true);
	bdd pos_factor = bdd_ite(Ixt, Txt, Ext);
	bdd Ixf = bdd_restrict(I, r, false);
	bdd Txf = bdd_restrict(T, r, false);
	bdd Exf = bdd_restrict(E, r, false);
	bdd neg_factor = bdd_ite(Ixf, Txf, Exf);
	r.high = &pos_factor.root;
	r.low = &neg_factor.root;
	printf("r index: %i", r.index);
	return bdd(r);
}

// Non-member Boolean operators on bdds
const bdd bdd_and(const bdd& input1, const bdd& input2) {
	return bdd_ite(input1, input2, bddfalse);
}

const bdd bdd_or(const bdd& input1, const bdd& input2) {
	return bdd_ite(input1, bddtrue, input2);
}

const bdd bdd_impl(const bdd& input1, const bdd& input2) {
	return bdd_ite(input1, input2, bddtrue);
}
///////////////////////////////////////////////////// END OF FRIEND FUNCTIONS //

// Constructors
bdd::bdd(const bdd& that) {
	this->root = that.root;
}
bdd::bdd(const bnode& that_root) {
	this->root = that_root;
}

// Boolean operators
const bdd bdd::operator&(const bdd& that) const {
	return bdd_and(*this, that);
}

const bdd bdd::operator|(const bdd& that) const {
	return bdd_or(*this, that);
}

const bdd bdd::operator>>(const bdd& that) const {
	return bdd_impl(*this, that);
}

// Boolean assignment operators
const bdd& bdd::operator&=(const bdd& that) {
	*this = bdd_and(*this, that);
	return *this;
}

const bdd& bdd::operator|=(const bdd& that) {
	*this = bdd_or(*this, that);
	return *this;
}

const bdd& bdd::operator>>=(const bdd& that) {
	*this = bdd_impl(*this, that);
	return *this;
}

// Comparison operators
bool bdd::operator==(const bdd& that) const {
	return (this->root == that.root);
}

// Output
std::ostream& operator<<(std::ostream& out, const bdd& btree) {
	return out << btree.root;
}

#endif
