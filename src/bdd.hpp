#ifndef YABDD_BDD_H
#define YABDD_BDD_H

#include <iostream>
#include <cmath>
#include <cstdio>
#include "assert.h"

#define PAIR(a,b)      ((unsigned int)((((unsigned int)a)+((unsigned int)b))*(((unsigned int)a)+((unsigned int)b)+((unsigned int)1))/((unsigned int)2)+((unsigned int)a)))
#define TRIPLE(a,b,c)  ((unsigned int)(PAIR((unsigned int)c,PAIR(a,b))))
#define NODEHASH(lvl, l, h)		(TRIPLE((lvl), (l), (h)) % bddnodesize);

int bddnodesize = 0;
int varsetsize = 0;
int numvars = 0;		// Number of variables actually used

class bdd {
public:
	int index; // lower index = closer to the top
	bdd* high; // pointer to the THEN node
	bdd* low; // pointer to the ELSE node
	unsigned int hash;

public:
	bdd(int i, bdd* h, bdd* l) {
		this->index = i;
		this->high = h;
		this->low = l;
		this->hash = NODEHASH(i, h, l);
	}

	bdd& operator=(bdd &that) {
		if (this != &that) {
			this->index = that.index;
			this->high = that.high;
			this->low = that.low;
			this->hash = that.hash;
		}
		return *this;
	}

	bool operator==(bdd& that) {
		return (this->index == that.index && this->high == that.high
				&& this->low == that.low);
	}
//	bool operator<(bdd& that) {
//		return (((that.index < 2) && (this->index > 1))
//				|| ((this->index > 1) && (this->index <= that.index)));
//	}
//
//	bool operator<=(bdd& that) {
//		return ((that.index < 2)
//				|| ((this->index > 1) && (this->index <= that.index)));
//	}

	// Boolean assignment operators
	bdd& operator&=(bdd& that) {
		*this = *(bdd_and(this, &that));
		return *this;
	}

	bdd& operator|=(bdd& that) {
		*this = *(bdd_or(this, &that));
		return *this;
	}

	bdd& operator>>=(bdd& that) {
		*this = *(bdd_impl(this, &that));
		return *this;
	}

private:
	friend int bdd_init(int initnodesize, int cachesize);
	friend int bdd_setnumvar(int num);

	friend bdd* bdd_makenode(int var, bdd* high, bdd* low);

	friend int bdd_var(int hash);
	friend bdd* bdd_high(int hash);
	friend bdd* bdd_low(int hash);

	friend bdd* bdd_ithvar(int i);

	friend bdd* bdd_restrict(bdd* subtree, int var, bool val);
	friend bdd* bdd_ite(bdd* I, bdd* T, bdd* E);

	friend bdd* bdd_and(bdd* lhs, bdd* rhs);
	friend bdd* bdd_or(bdd* lhs, bdd* rhs);
	friend bdd* bdd_impl(bdd* lhs, bdd* rhs);

	friend bdd satone(bdd* subtree);
	friend double satcount(bdd* subtree);
	friend std::ostream& operator<<(std::ostream& out, bdd& node);

	friend void bdd_done();
};

bdd** BDDTable; /* T: u -> (i, h, l) */

int bdd_init(int initnodesize, int cachesize) {
	bddnodesize = initnodesize;
	BDDTable = new bdd*[initnodesize];
	return 0;
}

int bdd_setnumvar(int num) {
	varsetsize = num;
	BDDTable[0] = new bdd(num, 0, 0);
	BDDTable[1] = new bdd(num, 0, 0);
	return 0;
}

bdd* bdd_false() {
	return BDDTable[0];
}

bdd& bdd_falsepp() {
	return *BDDTable[0];
}

bdd* bdd_true() {
	return BDDTable[1];
}

bdd& bdd_truepp() {
	return *BDDTable[1];
}

bdd* bdd_makenode(int var, bdd* high, bdd* low) {
	unsigned int hash = NODEHASH(var, high, low);
	if (BDDTable[hash] == 0) {
		BDDTable[hash] = new bdd(var, high, low);
	}
	return BDDTable[hash];
}

int bdd_var(int hash) {
	return BDDTable[hash]->index;
}

bdd* bdd_high(int hash) {
	return BDDTable[hash]->high;
}

bdd* bdd_low(int hash) {
	return BDDTable[hash]->low;
}

bdd* bdd_ithvar(int i) {
	numvars++;
	BDDTable[0]->index = numvars;
	BDDTable[1]->index = numvars;
	return bdd_makenode(i + 1, bdd_true(), bdd_false());
}

/** Shannon co-factor expansion wrt variable v (represented by bdd node)
 node index should always be at most btree root index */
bdd* bdd_restrict(bdd* subtree, int var, bool val) {
	if (subtree->index > var) {
		return subtree;
	} else if (subtree->index < var) {
		return bdd_makenode(subtree->index,
				bdd_restrict(subtree->high, var, val),
				bdd_restrict(subtree->low, var, val));
	} else { /* (subtree->index == var) */
		if (val) {
			return bdd_restrict(subtree->high, var, val);
		} else {
			return bdd_restrict(subtree->low, var, val);
		}
	}
}

// Non-member If-then-else BDD operator
bdd* bdd_ite(bdd* I, bdd* T, bdd* E) {
	// Base cases
	if (I == bdd_true())
		return T;
	if (I == bdd_false())
		return E;
	if (T == E)
		return T;
	if (T == bdd_true() && E == bdd_false())
		return I;

	// General cases
	// splitting variable, must be the topmost one among the root
	int split_var = I->index;
	if (split_var > T->index) {
		split_var = T->index;
	}
	if (split_var > E->index) {
		split_var = E->index;
	}
//	printf("%i %i %i %i\n", I.root.index, T.root.index, E.root.index, r.index);
	bdd* Ixt = bdd_restrict(I, split_var, true);
	bdd* Txt = bdd_restrict(T, split_var, true);
	bdd* Ext = bdd_restrict(E, split_var, true);
	bdd* pos_factor = bdd_ite(Ixt, Txt, Ext);
	bdd* Ixf = bdd_restrict(I, split_var, false);
	bdd* Txf = bdd_restrict(T, split_var, false);
	bdd* Exf = bdd_restrict(E, split_var, false);
	bdd* neg_factor = bdd_ite(Ixf, Txf, Exf);
	bdd* result = bdd_makenode(split_var, pos_factor, neg_factor);
	return result;
}

// Non-member Boolean operators on bdds
bdd* bdd_and(bdd* lhs, bdd* rhs) {
	return bdd_ite(lhs, rhs, bdd_false());
}

bdd* bdd_or(bdd* lhs, bdd* rhs) {
	return bdd_ite(lhs, bdd_true(), rhs);
}

bdd* bdd_impl(bdd* lhs, bdd* rhs) {
	return bdd_ite(lhs, rhs, bdd_true());
}

bdd* bdd_not(bdd* rhs) {
	return bdd_makenode(rhs->index, rhs->low, rhs->high);
}

bdd& operator&(bdd& lhs, bdd& rhs) {
	return *bdd_and(&lhs, &rhs);
}

bdd& operator|(bdd& lhs, bdd& rhs) {
	return *bdd_or(&lhs, &rhs);
}

bdd& operator>>(bdd& lhs, bdd& rhs) {
	return *bdd_impl(&lhs, &rhs);
}

bdd& operator!(bdd& rhs) {
	return *bdd_not(&rhs);
}

/* Insertion operator */
std::ostream& operator<<(std::ostream& out, bdd& subtree) {
	if (subtree.high || subtree.low) {
		out << "(";
	}
	out << subtree.index;
	if (subtree.high) {
		out << " " << *(subtree.high);
	}
	if (subtree.low) {
		out << " " << *(subtree.low);
	}
	if (subtree.high || subtree.low) {
		out << ")";
	}
	return out;
}

void bdd_fprintdot_rec(FILE* ofile, bdd* r) {
	if (r == bdd_true() || r == bdd_false()) {
		return;
	}
	fprintf(ofile, "%d [label=\"x%d\"];\n", ((unsigned int) r), r->index);
	fprintf(ofile, "%d -> %d [style=dotted];\n", ((unsigned int) r),
			((unsigned int) r->low));
	fprintf(ofile, "%d -> %d [style=filled];\n", ((unsigned int) r),
			((unsigned int) r->high));

	bdd_fprintdot_rec(ofile, r->low);
	bdd_fprintdot_rec(ofile, r->high);
}

void bdd_fprintdot(char* path, bdd* r) {
	FILE* ofile = fopen(path, "w+");
	fprintf(ofile, "digraph G {\n");
	fprintf(ofile,
			"%d [shape=box, label=\"0\", style=filled, shape=box, height=0.3, width=0.3];\n",
			((unsigned int) bdd_false()));
	fprintf(ofile,
			"%d [shape=box, label=\"1\", style=filled, shape=box, height=0.3, width=0.3];\n",
			((unsigned int) bdd_true()));

	bdd_fprintdot_rec(ofile, r);

	fprintf(ofile, "}\n");
	fclose(ofile);
}

void bdd_done() {
	delete[] BDDTable;
}

bdd* bdd_satone(bdd* subtree) {
	if (subtree == bdd_false()) {
		return 0;
	} else if (subtree == bdd_true()) {
		return subtree;
	} else if (subtree->low == bdd_false()) {
		return bdd_ite(subtree, bdd_true(), bdd_satone(subtree->high));
	} else {
		return bdd_ite(subtree, bdd_false(), bdd_satone(subtree->low));
	}
}

double bdd_satcount_rec(bdd* subtree) {
	if (subtree == bdd_false()) {
		return 0;
	} else if (subtree == bdd_true()) {
		return 1;
	} else {
		bdd* low = subtree->low;
		bdd* high = subtree->high;
		return pow(low->index - subtree->index - 1, 2) * bdd_satcount_rec(low)
				+ pow(high->index - subtree->index - 1, 2)
						* bdd_satcount_rec(high);
	}
}

double bdd_satcount(bdd* subtree) {
	return pow(subtree->index - 1, 2) * bdd_satcount_rec(subtree);
}

#endif
