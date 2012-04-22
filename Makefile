CXX = g++
CXXFLAGS = -Wall -ansi
LDFLAGS = 

SRC = ./src
OBJ = ./obj
BIN = ./bin

DEMO_TGT = $(BIN)/queens

src = $(wildcard $(SRC)/*.cpp)
obj = $(patsubst %.cpp, %.o, $(src))

.SUFFIXES = .h .c .cpp .o
.PHONY = all clean demo

all: demo

demo: $(obj)
	$(CXX) $(CXXFLAGS) -o $(DEMO_TGT) $(obj) $(LDFLAGS)

clean:
	-rm -rf $(OBJ)

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -o $(OBJ)/$@ -c $< $(LDFLAGS)


