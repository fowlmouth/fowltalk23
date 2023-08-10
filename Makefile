include config.mk

ALL: vm compiler 
.PHONY: clean cleanall

vm: src/vm/vm
	cp src/vm/vm ./

src/vm/vm: $(wildcard src/vm/*.cpp)
	make -C src/vm

libfowl.so: src/lib/libfowl.so
	cp src/lib/libfowl.so ./

src/lib/libfowl.so: $(wildcard src/lib/*.cpp)
	make -C src/lib

compiler: src/compiler/compiler src/lib/libfowl.so
	cp src/compiler/compiler ./

src/compiler/compiler: $(wildcard src/compiler/*.cpp)
	make -C src/compiler


clean:
	rm -f vm libfowl.so compiler

cleanall: clean
	make -C src/vm clean
	make -C src/lib clean
	make -C src/compiler clean

