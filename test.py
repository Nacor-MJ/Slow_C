import subprocess
from test_mod import *

test_all = False

import sys
if "all" in sys.argv:
    test_all = True

print(color + "-------------- Compiling MyCompiler" + end)
subprocess.run("g++ main.c -o main -ansi -pedantic -Wall -Wextra".split()).check_returncode()

print(color + "--------TESTS--------" +end);
compile_assert("idfk", "4 / 4", 1)
exit(1)
if not compile_assert("brackets", "(-6 + 16) * 3 - 2", 28, True):
    test_binops()

test_file("idk.txt")
if not compile_assert("basic comp", "3 > 5", False, True):
    test_comp_operators()

if test_all:
    print("all test")
    test_comp_operators()
    test_binops()

