all: .tinyap.built

.tinyap.built: src/*.cc src/*.c src/*.h src/Makefile
	+rm -f $@ && (cd src && make) && touch $@

install: .tinyap.built
	+cd src && make install

clean:
	+cd src && make clean
