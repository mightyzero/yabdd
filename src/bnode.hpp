#ifndef YABDD_BNODE_H
#define YABDD_BNODE_H

#include <iostream>
#include "bdd_op.hpp"
//~ enum bdd_op;			// forward declaration

struct bnode {
public:
	int 	index;		// lower index = closer to the top
	bnode* 	high;		// pointer to the THEN node
	bnode* 	low;		// pointer to the ELSE node

public:
	bnode();
	bnode(int i, bnode* h, bnode* l);		// Parameterized constructor
	bnode& operator=(const bnode &that);	// Copy constructor
	
	const bnode& apply(const bnode& that, const bdd_op op);	// Map op to ITE form
	//~ bool operator==(const bnode& that) const;			// Equality operator
	// Insertion operator
	friend std::ostream& operator<<(std::ostream& out, const bnode& node);
};

const bnode bddfalse(0, 0, 0);	// 0-sink
const bnode bddtrue(1, 0, 0);	// 1-sink


#endif
