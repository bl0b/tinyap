#!/bin/bash
./tinyap -pg > loop_test_1.txt
./tinyap -pg|./tinyap -i - -pag -pg > loop_test_2.txt
./tinyap -pg|./tinyap -i - -pag -pg|./tinyap -i - -pag -pg > loop_test_3.txt

diff loop_test_1.txt loop_test_2.txt && diff loop_test_1.txt loop_test_3.txt && diff loop_test_2.txt loop_test_3.txt && echo "print-grammar parsed successfully into itself"
exit $?
