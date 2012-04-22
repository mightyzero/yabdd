#ifndef YABDD_BDD_H
#define YABDD_BDD_H

enum BOP {
    AND,
    XOR,
    OR,
    NAND,
    NOR,
    IMP,
    BIIMP,
    DIFF,
    LESS,
    INVIMP
}

typedef struct BNode {
    int id;
    BNode* low;
    BNode* high;
} bnode;

typedef class BDD {
    BDD apply(BDD *that, BOP op);
        
} bdd;

BDD BDD::apply(BDD *that, BOP op) {
    
}

#endif
