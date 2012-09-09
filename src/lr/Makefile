CSRC=ape_prettyprint.c bootstrap.c parse_context.c pilot_manager.c serialize.c stack.c token_utils.c walkableast.c walker.c
CPPSRC=lr.cc ast.cc tinyap.cc unparser.cc trie.cc tinyap_alloc.cc string_registry.cc

COMMON_SRC = $(CSRC) $(CPPSRC)

COBJ=$(addprefix $(SUFFIX)/, $(subst .c,.o,$(CSRC)))
CPPOBJ=$(addprefix $(SUFFIX)/, $(subst .cc,.o,$(CPPSRC)))

LR_HEADERS= lr_base.h lr_equal_to.h lr_grammar.h lr_gss.h lr.h lr_less_than.h lr_visitors.h              
HEADERS=ast.h bootstrap.h config.h hashtab.h list.h parse_context.h pilot_cache.h pilot_manager.h serialize.h stack.h string_registry.h tinyap_alloc.h tinyape.h tinyap.h token_utils.h trie.h walkableast.h walker.h 
#PROF=-pg
#DBG=-ggdb
#OPT=-O3

ALLOC=-DUSE_MALLOC

CFLAGS=$(DBG) $(OPT) $(ALLOC) -fPIC -Wall $(PROF)
CPPFLAGS=$(DBG) $(OPT) $(ALLOC) -fPIC -Wno-deprecated -Wall $(PROF)

TARGET=$(SUFFIX)/tinyap
TEST=$(SUFFIX)/test
LIB_TARGET=$(SUFFIX)/libtinyap.so

CC=gcc $(CFLAGS)
CXX=g++ -I $(SUFFIX) $(CPPFLAGS)
LD=g++ -rdynamic $(DBG) $(PROF)
LDSHARED=$(LD) -shared
LIBS=-ldl -lpcre

all: flavours

-include .depend-$(SUFFIX)

test: debug
	./debug/test

clean:
	rm -f *.o */*.o $(TARGET) */$(TARGET) $(TEST) */$(TEST) .depend

targets: $(TARGET) $(TEST) $(LIB_TARGET)

$(TARGET): $(SUFFIX)/lr.h.gch $(COBJ) $(CPPOBJ) $(SUFFIX)/main.o
	$(LD) $(COBJ) $(CPPOBJ) $(SUFFIX)/main.o $(LIBS) -o $@

$(TEST): $(SUFFIX)/lr.h.gch $(COBJ) $(CPPOBJ) $(SUFFIX)/test.o
	$(LD) $(COBJ) $(CPPOBJ) $(SUFFIX)/test.o $(LIBS) -o $@

$(LIB_TARGET): $(SUFFIX)/lr.h.gch $(COBJ) $(CPPOBJ)
	$(LDSHARED) $(COBJ) $(CPPOBJ) $(LIBS) -o $@

.depend: $(HEADERS) $(LR_HEADERS) $(CSRC) $(CPPSRC) main.cc test.cc Makefile
	$(CC) --depend $(CSRC) > $@
	$(CXX) --depend $(CPPSRC) main.cc test.cc >> $@

.depend-$(SUFFIX): .depend
	sed "s,^.*:,$(SUFFIX)/&," $< > $@

$(COBJ):$(SUFFIX)/%.o:%.c Makefile
	$(CC) -c $< -o $@

$(SUFFIX)/lr.h.gch: $(LR_HEADERS) Makefile
	$(CXX) lr.h -o $@

$(SUFFIX)/test.o $(SUFFIX)/main.o $(CPPOBJ):$(SUFFIX)/%.o:%.cc Makefile
	$(CXX) -c $< -o $@

debug: FORCE
	mkdir -p $@
	+make DBG=-ggdb SUFFIX=$@ targets

opt: FORCE test
	mkdir -p $@
	+make OPT=-O3 SUFFIX=$@ targets
	cp $@/$(TARGET) $@/$(TEST) .

profile: FORCE test
	mkdir -p $@
	+make PROF=-pg\ -fprofile-arcs\ -ftest-coverage\ --coverage DBG=-g SUFFIX=$@ targets

opt-profile: FORCE test
	mkdir -p $@
	+make OPT=-O3 PROF=-pg SUFFIX=$@ targets


FLAVOURS = debug opt profile opt-profile

flavours: $(FLAVOURS)

## one-liner so make -j won't f* up
#flavours:
#	+make debug && make opt && make profile && make opt-profile && make clean

FORCE:

install:
	mkdir -p /usr/local/lib/tinyap
	mkdir -p /usr/local/include/tinyap
	cp -v opt/libtinyap.so /usr/local/lib/tinyap
	rm -f /usr/local/include/tinyap/*.h
	cp -v *.h /usr/local/include/tinyap
	cp opt/tinyap /usr/local/bin
