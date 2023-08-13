include ../../config.mk

BIN := vm

SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)

LIBFOWL := libfowl.a
LIBFOWL_FULL := ../lib/$(LIBFOWL)

CXXFLAGS += -I../lib
LDLIBS := -L../lib -lfowl

ALL: $(BIN)

$(BIN): $(OBJ) ../lib/libfowl.so
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

clean:
	rm -rf $(OBJ) $(BIN)

../lib/libfowl.so:
	make -C ../lib libfowl.so
