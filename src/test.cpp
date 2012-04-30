#include <iostream>

#include "bdd.hpp"

int main(int argc, char** argv) {
	bdd_init(100, 100);
	bdd_setnumvar(50);
	bdd* a = bdd_true();
	std::cout << *a << "\n";
	std::cout << (a == bdd_true()) << "\n";
	
	bdd* b = bdd_false();
	std::cout << *b << "\n";
	std::cout << (b == bdd_false()) << "\n";
	
	bdd* c = bdd_ithvar(0);
	bdd* d = bdd_ithvar(1);
	c = bdd_and(c, d);
	std::cout << *c << "\n";
	
	bdd* sol = bdd_satone(c);
	double numsol = bdd_satcount(c);

	bdd_fprintdot("test.dot", sol);
	printf("Number of solutions: %f", numsol);

	return 0;
}
