#include <iostream>

#include "bnode.hpp"
#include "bdd.hpp"

int main(int argc, char** argv) {
	bdd a = bddtrue;
	std::cout << a << "\n";
	std::cout << (a == bddtrue) << "\n";
	
	bdd b = bddfalse;
	std::cout << b << "\n";
	std::cout << (b == bddfalse) << "\n";
	
	bdd c = bdd_ithvar(0);
	bdd d = bdd_ithvar(1);
	c &= d;
	std::cout << c << "\n";
	
	return 0;
}
