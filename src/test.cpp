#include <iostream>

#include "bnode.hpp"
#include "bdd.hpp"

int main(int argc, char** argv) {
	bnode a = bddtrue;
	std::cout << a << "\n";
	
	bnode b = bddfalse;
	std::cout << b << "\n";
	
	bdd c = bnode(2, &a, &b);
	bdd d = bnode(3, &a, &b);
	c &= d;
	std::cout << c << "\n";
	
	return 0;
}
