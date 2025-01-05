include ../../config.mk

BIN := compiler

SRC := $(wildcard *.cpp) $(filter-out ../vm/main.cpp,$(wildcard ../vm/*.cpp))
OBJ := $(SRC:.cpp=.o)

CXXFLAGS += -I../lib -I../vm
CXXFLAGS += -g -O0 -DDEBUG
LDLIBS := -L../lib -lfowl

ALL: $(BIN)

$(BIN): ../lib/libfowl.so $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

../lib/libfowl.so: $(wildcard ../lib/*.o)
	$(MAKE) -C ../lib libfowl.so

clean:
	rm -rf $(OBJ) $(BIN)
