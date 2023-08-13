include config.mk

ALL: vm compiler 
.PHONY: clean cleanall

vm: src/vm/vm
	cp src/vm/vm ./

src/vm/vm: $(wildcard src/vm/*.cpp)
	$(MAKE) -C src/vm

libfowl.so: src/lib/libfowl.so
	cp src/lib/libfowl.so ./

src/lib/libfowl.so: $(wildcard src/lib/*.cpp)
	$(MAKE) -C src/lib debug

compiler: src/compiler/compiler src/lib/libfowl.so
	cp src/compiler/compiler ./

src/compiler/compiler: $(wildcard src/compiler/*.cpp)
	$(MAKE) -C src/compiler


clean:
	rm -f vm libfowl.so compiler

cleanall: clean
	$(MAKE) -C src/vm clean
	$(MAKE) -C src/lib clean
	$(MAKE) -C src/compiler clean

