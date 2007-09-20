OBJECTS=tinyap.o ast.o bootstrap.o tokenizer.o serialize.o node_cache.o walker.o pilot_manager.o ape_prettyprint.o stack.o walkableast.o
SOURCES=$(subst .o,.c,$(OBJECTS))

LIB_TARGET=libtinyap.so

TEST_C_TGT=tinyap
TEST_CPP_TGT=tinyap++

TEST_TARGETS=$(TEST_C_TGT) $(TEST_CPP_TGT) $(TEST_C_TGT)_static ape_prettyprint2.so ape_tinycalc.so

#CARGS=-Wall -ggdb
CCARGS=-Wall -ggdb -pg -fPIC

#CCARGS=-Wall -O3 -fPIC
#CCARGS=-Wall -O3

CC=gcc
CXX=g++
C=$(CC) $(CCARGS) $(CADD)
CX=$(CXX) $(CCARGS) $(CADD)

LD=gcc
LD_SHARE=-shared
LDARGS=-pg
LIBS=-Wl,--export-dynamic -ldl
L=$(LD) $(LDARGS) $(LIBS)

all: $(LIB_TARGET) $(TEST_TARGETS)

.depend: $(SOURCES)
	$C --depend $(SOURCES) > $@

include .depend

java:
	(cd Java&&make)

$(OBJECTS):%.o:%.c
	$C -c $< -o $@

$(LIB_TARGET): .depend $(OBJECTS)
	$L $(LD_SHARE) $(OBJECTS) -o $@

$(TEST_C_TGT): main.c
	$C $(LIB_TARGET) $< $(LIBS) -o $@

$(TEST_CPP_TGT): main++.c++
	$(CX) $(LIB_TARGET) $< $(LIBS) -o $@


$(TEST_C_TGT)_static: main.c
	$C $(OBJECTS) $< $(LIBS) -o $@

$(TEST_CPP_TGT)_static: main++.c++
	$C $(OBJECTS) $< $(LIBS) -o $@

ape_prettyprint2.so: ape_prettyprint2.c
	$C $(LD_SHARE) $< -o $@

ape_tinycalc.so: ape_tinycalc.c
	$C $(LD_SHARE) $< -o $@

clean:
	rm -f *~ *.o $(TEST_TARGETS) .depend
	(cd Java&&make clean)

.PHONY: doc clean

.explicit.grammar: tinyap libtinyap.so
	LD_LIBRARY_PATH=. ./tinyap -g explicit -pg > .explicit.grammar

.CamelCasing.grammar: tinyap libtinyap.so
	LD_LIBRARY_PATH=. ./tinyap -g CamelCasing -pg > .CamelCasing.grammar

doc: Doxyfile .explicit.grammar .CamelCasing.grammar tinyap.h bootstrap.h
	rm -rf doc/
	doxygen
