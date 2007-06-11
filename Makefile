OBJECTS=tinyap.o ast.o bootstrap.o tokenizer.o serialize.o main.o
SOURCES=$(subst .o,.c,$(OBJECTS))

#CARGS=-Wall -ggdb

CCARGS=-Wall -ggdb
#CCARGS=-Wall -O3
CC=gcc
C=$(CC) $(CCARGS) $(CADD)

LD=gcc
LDARGS=
L=$(LD) $(LDARGS)

all: tinyap java

.depend: $(SOURCES)
	$C --depend $(SOURCES) > $@

include .depend

java:
	(cd Java&&make)

$(OBJECTS):%.o:%.c
	$C -c $< -o $@

tinyap: .depend $(OBJECTS)
	$L $(OBJECTS) -o tinyap

clean:
	rm -f *~ *.o tinyap .depend
	(cd Java&&make clean)

