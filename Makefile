OBJECTS=tinyap.o ast.o bootstrap.o tokenizer.o serialize.o
SOURCES=$(subst .o,.c,$(OBJECTS))

LIB_TARGET=libtinyap.so

TEST_C_TGT=tinyap
TEST_CPP_TGT=tinyap++

TEST_TARGETS=$(TEST_C_TGT) $(TEST_CPP_TGT)

#CARGS=-Wall -ggdb

CCARGS=-Wall -ggdb
#CCARGS=-Wall -O3
CC=gcc
CXX=g++
C=$(CC) $(CCARGS) $(CADD)
CX=$(CXX) $(CCARGS) $(CADD)

LD=gcc
LD_SHARE=-shared
LDARGS=
L=$(LD) $(LDARGS)

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
	$C $(LIB_TARGET) $< -o $@

$(TEST_CPP_TGT): main++.c++
	$(CX) $(LIB_TARGET) $< -o $@


clean:
	rm -f *~ *.o tinyap .depend
	(cd Java&&make clean)

.PHONY: doc clean

.explicit.grammar: tinyap libtinyap.so
	LD_LIBRARY_PATH=. ./tinyap -g explicit -pg > .explicit.grammar

.CamelCasing.grammar: tinyap libtinyap.so
	LD_LIBRARY_PATH=. ./tinyap -g CamelCasing -pg > .CamelCasing.grammar

doc: Doxyfile .explicit.grammar .CamelCasing.grammar tinyap.h bootstrap.h
	rm -rf doc/
	doxygen
