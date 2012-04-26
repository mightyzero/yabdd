#ifndef YABDD_BDD_OP_H
#define YABDD_BDD_OP_H

/* Enumerate the operations that can be performed on bdds */
enum bdd_op {
	AND,
	XOR,
	OR,
	NAND,
	NOR,
	IMP,
	BIIMP,
	INVIMP
};

#endif
