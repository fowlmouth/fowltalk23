BIN := compiler-tests

SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)

CO_SRC := $(wildcard ../../src/compiler/*.cpp)
CO_OBJ := $(filter-out co__main.o,$(patsubst ../../src/compiler/%.cpp,co__%.o,$(CO_SRC)))

OBJ += $(CO_OBJ)

CXXFLAGS := -std=c++17 -I../ -I../../src/compiler -I../../src/vm -I../../src/lib
LDFLAGS := -L../../src/lib -lfowl

ALL: $(BIN)
.PHONY: clean cleanall

$(BIN): $(OBJ) libfowl.so
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(BIN) $(LDFLAGS)
co__%.o: ../../src/compiler/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

libfowl.so: ../../src/lib/libfowl.so
	cp $< .
../../src/lib/libfowl.so:
	$(MAKE) -C ../../src/lib libfowl.so

clean:
	rm -f $(OBJ)
cleanall: clean
	rm -f $(BIN)
