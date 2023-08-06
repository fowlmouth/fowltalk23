include ../../config.mk

LIB := libfowl.a
SONAME := libfowl.so

SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)

ALL: $(SONAME) $(LIB)

$(SONAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

clean:
	rm -rf $(OBJ) $(LIB) $(SONAME)
