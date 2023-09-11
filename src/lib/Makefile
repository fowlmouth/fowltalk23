include ../../config.mk

LIB := libfowl.a
SONAME := libfowl.so

SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)

CXXFLAGS += -fPIC

ALL: $(SONAME) $(LIB)
.PHONY: debug clean

debug: CXXFLAGS += -DDEBUG -g -O0
debug: $(SONAME)

$(SONAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

clean:
	rm -rf $(OBJ) $(LIB) $(SONAME)
