CXX = g++
CXXFLAGS = -Wall -ansi
LDFLAGS = 

SRC = ./src

demo = queens
demo_obj = $(SRC)/$(demo).o
test = test
test_obj = $(SRC)/$(test).o

src = $(SRC)/bdd.cpp $(SRC)/bnode.cpp
obj = $(patsubst %.cpp, %.o, $(src))

.SUFFIXES = .hpp .cpp .o
.PHONY = all clean demo test

all: test demo

test: $(test_obj) $(obj)
	-rm -f test.dot
	$(CXX) $(CXXFLAGS) -o $(test) $(test_obj) $(obj) $(LDFLAGS)
	-./$(test)
	-dot test.dot
	-dotty test.dot

demo: $(demo_obj) $(obj)
	-rm -f queens.dot
	$(CXX) $(CXXFLAGS) -o $(demo) $(demo_obj) $(obj) $(LDFLAGS)
	-./$(demo) 8
	-dot queens.dot
	-dotty queens.dot

clean:
	-rm -f $(test)
	-rm -f $(demo)
	-rm -f $(SRC)/*.o
	-rm -f *.dot

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -o $@ -c $< $(LDFLAGS)


