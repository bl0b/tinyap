OBJECTS=ast.o bootstrap.o tokenizer.o serialize.o main.o
SOURCES=$(subst .o,.c,$(OBJECTS))

#CARGS=-Wall -ggdb
CCARGS=-Wall -O3
CC=gcc
C=$(CC) $(CCARGS) $(CADD)

LD=gcc
LDARGS=
L=$(LD) $(LDARGS)

all: standalone java

java:
	(cd Java&&make)

$(OBJECTS):%.o:%.c
	$C -c $< -o $@

standalone: $(OBJECTS)
	$L $(OBJECTS) -o tinyap

clean:
	rm -f *~ *.o tinyap
	(cd Java&&make clean)

