include ../../config.mk

BIN := vm

SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)

LIBFOWL := libfowl.a
LIBFOWL_FULL := ../lib/$(LIBFOWL)

CXXFLAGS += -I../lib
LDLIBS := -L../lib #$(LIBFOWL_FULL)

ALL: $(BIN)

$(BIN): $(OBJ) $(LIBFOWL_FULL)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -rf $(OBJ) $(BIN)

$(LIBFOWL_FULL):
	make -C ../lib $(LIBFOWL)
