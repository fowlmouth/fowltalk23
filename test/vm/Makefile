BIN := vm-tests

SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)

VM_SRC := $(wildcard ../../src/vm/*.cpp)
VM_OBJ := $(filter-out vm__main.o,$(patsubst ../../src/vm/%.cpp,vm__%.o,$(VM_SRC)))

OBJ += $(VM_OBJ)

CXXFLAGS := -std=c++17 -I../ -I../../src/vm -I../../src/lib
LDFLAGS := -L../../src/lib -lfowl

ALL: $(BIN)
.PHONY: clean cleanall

$(BIN): $(OBJ) libfowl.so
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(BIN) $(LDFLAGS)
vm__%.o: ../../src/vm/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

libfowl.so: ../../src/lib/libfowl.so
	cp $< .
../../src/lib/libfowl.so:
	$(MAKE) -C ../../src/lib libfowl.so

clean:
	rm -f $(OBJ)
cleanall: clean
	rm -f $(BIN)
