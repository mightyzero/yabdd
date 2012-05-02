#ifndef YABDD_BDD_H
#define YABDD_BDD_H

#include <iostream>
#include <cmath>
#include <cstdio>
#include <assert.h>

#define PAIR(a,b)      ((unsigned int)((((unsigned int)a)+((unsigned int)b))*(((unsigned int)a)+((unsigned int)b)+((unsigned int)1))/((unsigned int)2)+((unsigned int)a)))
#define TRIPLE(a,b,c)  ((unsigned int)(PAIR((unsigned int)c,PAIR(a,b))))
#define NODEHASH(lvl, l, h)     (TRIPLE((lvl), (l), (h)) % bddnodesize);

int bddnodesize = 0;
int numvars = 0;		// Number of variables actually used

struct bdd {
public:
    int index;          // lower index = closer to the top
    bdd* high;          // pointer to the THEN bdd
    bdd* low;           // pointer to the ELSE bdd
    bdd* next;          // pointer to the next bdd in the bucket

public:
    bdd(int i, bdd* h, bdd* l) {
        this->index = i;
        this->high = h;
        this->low = l;
        this->next = 0;
    }

    bdd& operator=(bdd &that) {
        if (this != &that) {
            this->index = that.index;
            this->high = that.high;
            this->low = that.low;
        }
        return *this;
    }

    bool operator==(bdd& that) {
        return (this->index == that.index && this->high == that.high
                && this->low == that.low);
    }

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
    friend int bdd_init(int initnodesize);
    friend int bdd_setnumvar(int num);

    friend bdd* bdd_makenode(int var, bdd* high, bdd* low);

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

int bdd_init(int initnodesize) {
    bddnodesize = initnodesize;
    BDDTable = new bdd*[initnodesize];
    BDDTable[0] = new bdd(numvars, 0, 0);
    BDDTable[1] = new bdd(numvars, 0, 0);
    return 0;
}

/* Return a pointer to the 0 sink */
bdd* bdd_false() {
    return BDDTable[0];
}

/* Return a pointer to the 1 sink */
bdd* bdd_true() {
    return BDDTable[1];
}

/* Return a reference (instead of a pointer) to the 0 sink */
bdd& bdd_falsepp() {
    return *BDDTable[0];
}

/* Return a reference (instead of a pointer) to the 1 sink */
bdd& bdd_truepp() {
    return *BDDTable[1];
}

/* Search for the specified triple in the node look-up table.
 * If found, return node.
 * Otherwise, make new node, add to table, and return
 */
bdd* bdd_makenode(int var, bdd* high, bdd* low) {
    unsigned int hash = NODEHASH(var, high, low);
    // no bucket found -> no matching node
    if (BDDTable[hash] == 0) {
        BDDTable[hash] = new bdd(var, high, low);
        return BDDTable[hash];
    } else { 
        bdd* node = BDDTable[hash];     // first node in bucket found
        // search bucket for the matching node
        while (node->next && (node->index != var || node->high != high || node->low != low))
            node = node->next;
        // no matching node in bucket
        if (node->index != var || node->high != high || node->low != low) {
            bdd* newNode = new bdd(var, high, low);  // make new node
            node->next = newNode;                     // add to bucket
            return newNode;
        } else {
            // found matching node in bucket
            return node;
        }
    }
}

/*
 * Return a bdd representing the ith input variable,
 * with the high node set to the 1 sink and the low node set to the 0 sink
 */
bdd* bdd_ithvar(int i) {
    numvars++;
    // adjust index of terminal nodes; satcount will fail otherwise
    BDDTable[0]->index = numvars;
    BDDTable[1]->index = numvars;
    return bdd_makenode(i, bdd_true(), bdd_false());
}

/* 
 * Return the Shannon co-factor expansion wrt variable var
 */
bdd* bdd_restrict(bdd* subtree, int var, bool val) {
    assert(subtree != 0);
    if (subtree->index > var) {
        return subtree;
    } else if (subtree->index < var) {
        return bdd_makenode(subtree->index,
                bdd_restrict(subtree->high, var, val),
                bdd_restrict(subtree->low, var, val));
    } else { /* (subtree->index == var) */
        if (val) {
            return subtree->high;
        } else {
            return subtree->low;
        }
    }
}

// Non-member If-then-else BDD operator
bdd* bdd_ite(bdd* I, bdd* T, bdd* E) {
    assert(I != 0);
    assert(T != 0);
    assert(E != 0);
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
    assert(lhs != 0);
    assert(rhs != 0);
    return bdd_ite(lhs, rhs, bdd_false());
}

bdd* bdd_or(bdd* lhs, bdd* rhs) {
    assert(lhs != 0);
    assert(rhs != 0);
    return bdd_ite(lhs, bdd_true(), rhs);
}

bdd* bdd_impl(bdd* lhs, bdd* rhs) {
    assert(lhs != 0);
    assert(rhs != 0);
    return bdd_ite(lhs, rhs, bdd_true());
}

bdd* bdd_not(bdd* rhs) {
    assert(rhs != 0);
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
    assert(ofile != 0);
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
    assert(r != 0);
    FILE* ofile = fopen(path, "w+");
    assert(ofile != 0);
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
    if (BDDTable)
        delete[] BDDTable;
}

bdd* bdd_satone_rec(bdd* subtree) {
    assert(subtree != 0);
    if (subtree == bdd_false() || subtree == bdd_true()) {
        return subtree;
    } else if (subtree->low == bdd_false()) {
        return bdd_makenode(subtree->index, bdd_false(),
                bdd_satone_rec(subtree->high));
    } else { /* subtree->high == bdd_false() */
        return bdd_makenode(subtree->index, bdd_satone_rec(subtree->low),
                bdd_false());
    }
}

bdd* bdd_satone(bdd* subtree) {
    assert(subtree != 0);
    if (subtree == bdd_false())
        return 0;
    else
        return bdd_satone_rec(subtree);
}

double bdd_satcount_rec(bdd* subtree) {
    assert(subtree != 0);
    if (subtree == bdd_false()) {
        return 0;
    } else if (subtree == bdd_true()) {
        return 1;
    } else {
        bdd* low = subtree->low;
        bdd* high = subtree->high;
        double size = 0.0, s = 1.0;
        s = pow(2.0, (low->index) - (subtree->index) - 1);
        size += s * bdd_satcount_rec(low);
        s = pow(2.0, (high->index) - (subtree->index) - 1);
        size += s * bdd_satcount_rec(high);
        return size;
    }
}

double bdd_satcount(bdd* subtree) {
    assert(subtree != 0);
    return pow(2.0, (subtree->index) - 1) * bdd_satcount_rec(subtree);
}

#endif
