include config.mk

ALL: vm compiler 
.PHONY: clean cleanall

vm: src/vm/vm libfowl.so
	cp src/vm/vm ./

src/vm/vm: $(wildcard src/vm/*.cpp)
	$(MAKE) -C src/vm debug

libfowl.so: src/lib/libfowl.so
	cp src/lib/libfowl.so ./

src/lib/libfowl.so: $(wildcard src/lib/*.cpp)
	$(MAKE) -C src/lib debug

compiler: src/compiler/compiler libfowl.so
	cp src/compiler/compiler ./

src/compiler/compiler: $(wildcard src/compiler/*.cpp)
	$(MAKE) -C src/compiler

test.img: compiler libfowl.so
	./compiler --file test/infix_message --output test.img

clean:
	rm -f vm libfowl.so compiler test.img

debug:
	$(MAKE) -C src/lib debug
	$(MAKE) -C src/compiler
	$(MAKE) compiler

cleanall: clean
	$(MAKE) -C src/vm clean
	$(MAKE) -C src/lib clean
	$(MAKE) -C src/compiler clean

