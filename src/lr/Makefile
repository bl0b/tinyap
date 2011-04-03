CSRC=ape_prettyprint.c bootstrap.c parse_context.c pilot_manager.c serialize.c stack.c string_registry.c tinyap_alloc.c token_utils.c trie.c walkableast.c walker.c
CPPSRC=lr.cc ast.cc tinyap.cc unparser.cc

COMMON_SRC = $(CSRC) $(CPPSRC)

COBJ=$(subst .c,.o,$(CSRC))
CPPOBJ=$(subst .cc,.o,$(CPPSRC))


PROF=-pg

CFLAGS=-ggdb -Wall $(PROF)
CPPFLAGS=-ggdb -Wno-deprecated -Wall $(PROF)

TARGET=tinyap
TEST=test

CC=gcc $(CFLAGS)
CXX=g++ $(CPPFLAGS)
LD=g++ -ldl -lpcre -rdynamic $(PROF)

all: $(TARGET) $(TEST)

-include .depend

clean:
	rm -f *.o $(TARGET) .depend

$(TARGET): $(COBJ) $(CPPOBJ) main.o
	$(LD) $(COBJ) $(CPPOBJ) main.o -o $@

$(TEST): $(COBJ) $(CPPOBJ) test.o
	$(LD) $(COBJ) $(CPPOBJ) test.o -o $@

.depend: $(CSRC) $(CPPSRC) main.c test.cc Makefile
	$(CC) --depend $(CSRC) main.c > $@
	$(CXX) --depend $(CPPSRC) test.cc >> $@

main.o $(COBJ):%.o:%.c Makefile
	$(CC) -c $< -o $@

test.o $(CPPOBJ):%.o:%.cc Makefile
	$(CXX) -c $< -o $@

