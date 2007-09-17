#!/bin/bash
gcc -ggdb -pg -Wall -Wl,--export-dynamic pilot_manager.c stack.c walkableast.c walker.c test.c test_pilot.c ape_prettyprint.c -ldl ../libtinyap.so
