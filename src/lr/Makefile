CSRC=ape_prettyprint.c bootstrap.c parse_context.c pilot_manager.c serialize.c stack.c string_registry.c tinyap_alloc.c token_utils.c walkableast.c walker.c
CPPSRC=lr.cc ast.cc tinyap.cc unparser.cc trie.cc

COMMON_SRC = $(CSRC) $(CPPSRC)

COBJ=$(addprefix $(SUFFIX)/, $(subst .c,.o,$(CSRC)))
CPPOBJ=$(addprefix $(SUFFIX)/, $(subst .cc,.o,$(CPPSRC)))

LR_HEADERS= lr_base.h lr_equal_to.h lr_grammar.h lr_gss.h lr.h lr_less_than.h lr_visitors.h              
HEADERS=ast.h bootstrap.h config.h hashtab.h list.h parse_context.h pilot_cache.h pilot_manager.h serialize.h stack.h string_registry.h tinyap_alloc.h tinyape.h tinyap.h token_utils.h trie.h walkableast.h walker.h 
#PROF=-pg
#DBG=-ggdb
#OPT=-O3

CFLAGS=$(DBG) $(OPT) -Wall $(PROF)
CPPFLAGS=$(DBG) $(OPT) -Wno-deprecated -Wall $(PROF)

TARGET=$(SUFFIX)/tinyap
TEST=$(SUFFIX)/test

CC=gcc $(CFLAGS)
CXX=g++ $(CPPFLAGS)
LD=g++ -ldl -lpcre -rdynamic $(PROF)

all: flavours

-include .depend-$(SUFFIX)

clean:
	rm -f {,*/}*.o {,*/}$(TARGET) {,*/}$(TEST) .depend

targets: $(TARGET) $(TEST)

$(TARGET): lr.h.gch $(COBJ) $(CPPOBJ) $(SUFFIX)/main.o
	$(LD) $(COBJ) $(CPPOBJ) $(SUFFIX)/main.o -o $@

$(TEST): lr.h.gch $(COBJ) $(CPPOBJ) $(SUFFIX)/test.o
	$(LD) $(COBJ) $(CPPOBJ) $(SUFFIX)/test.o -o $@

.depend: $(HEADERS) $(LR_HEADERS) $(CSRC) $(CPPSRC) main.cc test.cc Makefile
	$(CC) --depend $(CSRC) > $@
	$(CXX) --depend $(CPPSRC) main.cc test.cc >> $@

.depend-$(SUFFIX): .depend
	sed "s,^.*:,$(SUFFIX)/&," $< > $@

$(COBJ):$(SUFFIX)/%.o:%.c Makefile
	$(CC) -c $< -o $@

lr.h.gch: $(LR_HEADERS) Makefile
	$(CXX) lr.h

$(SUFFIX)/test.o $(SUFFIX)/main.o $(CPPOBJ):$(SUFFIX)/%.o:%.cc Makefile
	$(CXX) -c $< -o $@

debug: FORCE
	mkdir -p $@
	+make DBG=-ggdb SUFFIX=$@ targets

opt: FORCE
	mkdir -p $@
	+make OPT=-O3 SUFFIX=$@ targets
	cp $@/$(TARGET) $@/$(TEST) .

profile: FORCE
	mkdir -p $@
	+make PROF=-pg DBG=-ggdb SUFFIX=$@ targets

opt-profile: FORCE
	mkdir -p $@
	+make OPT=-O3 PROF=-pg SUFFIX=$@ targets


FLAVOURS = debug opt profile opt-profile

flavours: $(FLAVOURS)

## one-liner so make -j won't f* up
#flavours:
#	+make debug && make opt && make profile && make opt-profile && make clean

FORCE:

