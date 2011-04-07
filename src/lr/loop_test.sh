#!/bin/bash
./tinyap-debug -pg > loop_test_1.txt
./tinyap-debug -pg|./tinyap-debug -i - -pag -pg > loop_test_2.txt
./tinyap-debug -pg|./tinyap-debug -i - -pag -pg|./tinyap-debug -i - -pag -pg > loop_test_3.txt

diff loop_test_1.txt loop_test_2.txt && diff loop_test_1.txt loop_test_3.txt && diff loop_test_2.txt loop_test_3.txt && echo "print-grammar parsed successfully into itself"
exit $?
