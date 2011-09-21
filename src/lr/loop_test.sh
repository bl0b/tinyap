#!/bin/bash
CMD=./debug/tinyap
$CMD -pg > loop_test_1.txt
$CMD -pg|$CMD -i - -pag -pg > loop_test_2.txt
$CMD -pg|$CMD -i - -pag -pg|$CMD -i - -pag -pg > loop_test_3.txt

diff loop_test_1.txt loop_test_2.txt && diff loop_test_1.txt loop_test_3.txt && diff loop_test_2.txt loop_test_3.txt && echo "print-grammar parsed successfully into itself"
exit $?
