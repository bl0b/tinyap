SUBDIRS = src examples
EXTRA_DIST=tinyap.pc.in

ACLOCAL_AMFLAGS=-I m4

tinyap.pc: tinyap.pc.in
	./gen_pc.pl tinyap.pc.in "$(prefix)" "$(VERSION)"

install: install-recursive tinyap.pc

