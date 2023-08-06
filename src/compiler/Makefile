BIN := compiler

SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)

CXXFLAGS := -I../lib -std=c++20
LDLIBS := -L../lib -lfowl

ALL: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -rf $(OBJ) $(BIN)
