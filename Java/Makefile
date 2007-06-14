TARGET=libtinyajp_Native.so

JAR_TARGET=tinyajp.jar

OBJECTS=../ast.o ../bootstrap.o ../tokenizer.o ../serialize.o ../tinyap.o
SOURCES=$(subst .o,.c,$(OBJECTS))

# java lacks pkg-config support
JINC=$(shell ./javaconfig.sh --include)
JAVAC=$(shell which javac||echo "erreur : pas de javac dans le PATH"&&exit -1)
JAVAH=$(shell which javah||echo "erreur : pas de javah dans le PATH"&&exit -1)
JAR=$(shell which jar||echo "erreur : pas de jar dans le PATH"&&exit -1)


JAR_SOURCES=Demo.java Interpreter.java Panneau.java
JAR_CLASSES=$(subst .java,.class,$(JAR_SOURCES))

JAR_FILES= \
	$(JAR_CLASSES) \
	$(JAR_LIB_CLASSES) \
	tinyajp/AstNode.class \
	tinyajp/Parser.class \
	tinyajp/Native.class


#CARGS=$(JINC) -ggdb
CARGS=$(JINC) -O3
CC=gcc
C=$(CC) $(CARGS) $(CADD)

LDARGS=-shared
LD=gcc
L=$(LD) $(LDARGS)

all: $(OBJECTS) $(JAR_TARGET)

$(OBJECTS):
	cd ..&&make $(subst ../,,$(OBJECTS))

tinyajp/Native.class: tinyajp/Native.java
	$(JAVAC) $<

tinyajp_Native.h: tinyajp/Native.class
	$(JAVAH) tinyajp.Native

$(TARGET):lib%.so: %.c %.h $(OBJECTS)
	$C -c $< -o $<.o
	$L $<.o $(OBJECTS) -o $@

$(JAR_LIB_CLASSES) $(JAR_CLASSES):%.class:%.java
	$(JAVAC) $<

.PHONY: clean

clean:
	rm -f $(OBJECTS) $(JAR_FILES) libparsing.so *.class *~ .*.sw? *.bak

mr-proper:

$(JAR_TARGET): $(JAR_FILES) $(TARGET)
	@rm -f $@
	$(JAR) cvf $@ -m $(JAR_FILES)
	

