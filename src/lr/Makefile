CSRC=ape_prettyprint.c bootstrap.c parse_context.c pilot_manager.c serialize.c stack.c string_registry.c tinyap_alloc.c tinyap.c token_utils.c trie.c walkableast.c walker.c
CPPSRC=lr.cc test.cc ast.cc

COBJ=$(subst .c,.o,$(CSRC))
CPPOBJ=$(subst .cc,.o,$(CPPSRC))


CFLAGS=-ggdb -Wall -O2 -pg
CPPFLAGS=-ggdb -Wno-deprecated -Wall -pg

TARGET=tinyap

CC=gcc $(CFLAGS)
CXX=g++ $(CPPFLAGS)
LD=g++ -ldl -lpcre -rdynamic -pg

all: $(TARGET)

-include .depend

clean:
	rm -f *.o $(TARGET)

$(TARGET): $(COBJ) $(CPPOBJ)
	$(LD) $(COBJ) $(CPPOBJ) -o $@

.depend: $(CSRC) $(CPPSRC) Makefile
	$(CC) --depend $(CSRC) > $@
	$(CXX) --depend $(CPPSRC) >> $@

$(COBJ): %.o: %.c Makefile
	$(CC) -c $< -o $@

$(CPPOBJ): %.o: %.cc Makefile
	$(CXX) -c $< -o $@

