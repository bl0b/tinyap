CSRC=ape_prettyprint.c bootstrap.c parse_context.c pilot_manager.c serialize.c stack.c string_registry.c tinyap_alloc.c token_utils.c trie.c walkableast.c walker.c
CPPSRC=lr.cc ast.cc tinyap.cc unparser.cc

COMMON_SRC = $(CSRC) $(CPPSRC)

COBJ=$(subst .c,.o,$(CSRC))
CPPOBJ=$(subst .cc,.o,$(CPPSRC))


#PROF=-pg
#DBG=-ggdb
#OPT=-O3

CFLAGS=$(DBG) $(OPT) -Wall $(PROF)
CPPFLAGS=$(DBG) $(OPT) -Wno-deprecated -Wall $(PROF)

TARGET=tinyap-$(SUFFIX)
TEST=test-$(SUFFIX)

CC=gcc $(CFLAGS)
CXX=g++ $(CPPFLAGS)
LD=g++ -ldl -lpcre -rdynamic $(PROF)

all: $(TARGET) $(TEST)

-include .depend

clean:
	rm -f *.o $(TARGET) .depend

$(TARGET): lr.h.gch $(COBJ) $(CPPOBJ) main.o
	$(LD) $(COBJ) $(CPPOBJ) main.o -o $@

$(TEST): lr.h.gch $(COBJ) $(CPPOBJ) test.o
	$(LD) $(COBJ) $(CPPOBJ) test.o -o $@

.depend: $(CSRC) $(CPPSRC) main.cc test.cc Makefile
	$(CC) --depend $(CSRC) > $@
	$(CXX) --depend $(CPPSRC) main.cc test.cc >> $@

$(COBJ):%.o:%.c Makefile
	$(CC) -c $< -o $@

lr.h.gch: lr.h Makefile
	$(CXX) lr.h

test.o main.o $(CPPOBJ):%.o:%.cc Makefile
	$(CXX) -c $< -o $@

debug:
	+make DBG=-ggdb SUFFIX=debug all

opt:
	+make OPT=-O3 SUFFIX=profile all

profile:
	+make PROF=-pg DBG=-ggdb SUFFIX=profile all

opt-profile:
	+make OPT=-O3 PROF=-pg SUFFIX=opt-profile all

# one-liner so make -j won't f* up
flavours:
	+make clean debug && make clean opt && make clean profile && make clean opt-profile && make clean

