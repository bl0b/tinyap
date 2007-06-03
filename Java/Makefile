TARGETS=libparsing_TinyaJP.so libparsing_AST.so

JAR_TARGET=tinyajp.jar

OBJECTS=../ast.o ../bootstrap.o ../tokenizer.o ../serialize.o
SOURCES=$(subst .o,.c,$(OBJECTS))

# java lacks pkg-config support
JINC=$(shell ./javaconfig.sh --include)
JAVAC=$(shell which javac||echo "erreur : pas de javac dans le PATH"&&exit -1)
JAVAH=$(shell which javah||echo "erreur : pas de javah dans le PATH"&&exit -1)
JAR=$(shell which jar||echo "erreur : pas de jar dans le PATH"&&exit -1)


JAR_SOURCES=Demo.java Interpreter.java Panneau.java
JAR_CLASSES=$(subst .java,.class,$(JAR_SOURCES))

JAR_LIB_SOURCES =$(TARGETS) libparsing.so
JAR_FILES= \
	$(JAR_CLASSES) \
	$(JAR_LIB_SOURCES) \
	$(JAR_LIB_CLASSES) \
	parsing/AST.class \
	parsing/TinyaJP.class


#CARGS=$(JINC) -ggdb
CARGS=$(JINC) -O3
CC=gcc
C=$(CC) $(CARGS) $(CADD)

LDARGS=-shared
LD=gcc
L=$(LD) $(LDARGS)

all: $(JAR_TARGET)

parsing/AST.class: parsing/AST.java
	$(JAVAC) $<

parsing/TinyaJP.class: parsing/TinyaJP.java
	$(JAVAC) $<

parsing_AST.h: parsing/AST.class
	$(JAVAH) parsing.AST

parsing_TinyaJP.h: parsing/TinyaJP.class
	$(JAVAH) parsing.TinyaJP

$(TARGETS):lib%.so: %.c %.h libparsing.so
	$C -c $< -o $<.o
	$L $<.o libparsing.so -o $@

libparsing.so: $(OBJECTS)
	$L $(OBJECTS) -o $@

$(JAR_LIB_CLASSES) $(JAR_CLASSES):%.class:%.java
	$(JAVAC) $<

.PHONY: clean

clean:
	rm -f $(OBJECTS) $(JAR_FILES) libparsing.so *.class *~ .*.sw? *.bak

mr-proper:

$(JAR_TARGET): $(JAR_FILES) parsing/AST$$Node.class Panneau$$HeadPhones.class
	@rm -f $@
	$(JAR) cvf $@ -m $(JAR_FILES) parsing/AST\$$Node.class Panneau\$$HeadPhones.class
	

