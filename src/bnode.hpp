#ifndef YABDD_BNODE_H
#define YABDD_BNODE_H

#include <iostream>
#include "bdd_op.hpp"
//~ enum bdd_op;			// forward declaration

struct bnode {
public:
	int index; // lower index = closer to the top
	int high;  // pointer to the THEN node
	int low; // pointer to the ELSE node

public:
	// Constructors
	bnode();
	bnode(int i);
	bnode(int i, int h, int l);
	bnode& operator=(const bnode &that); // Copy constructor

	bool operator==(const bnode& that) const;
	bool operator<(const bnode& that) const;
	bool operator<=(const bnode& that) const;

	const bnode& apply(const bnode& that, const bdd_op op); // Map op to ITE form

private:
	friend std::ostream& operator<<(std::ostream& out, const bnode& node);
};

const bnode bnode_0(0, 0, 0); // 0-sink
const bnode bnode_1(1, 0, 0); // 1-sink

const bnode operator !(const bnode& node);
const bnode& bnode_min(const bnode& a, const bnode& b);
const bnode& bnode_min(const bnode& a, const bnode& b, const bnode& c);

#endif
