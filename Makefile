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

all: demo

test: $(test_obj) $(obj)
	$(CXX) $(CXXFLAGS) -o $(test) $(test_obj) $(obj) $(LDFLAGS)
	-./$(test)

demo: $(demo_obj).o $(obj)
	$(CXX) $(CXXFLAGS) -o $(demo) $(demo_obj) $(obj) $(LDFLAGS)
	-./$(demo)

clean:
	-rm -f $(obj)

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -o $@ -c $< $(LDFLAGS)


