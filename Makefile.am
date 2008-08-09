SUBDIRS = src examples

tinyap.pc: tinyap.pc.in
	./gen_pc.pl tinyap.pc.in "$(prefix)" "$(VERSION)"

install: tinyap.pc

