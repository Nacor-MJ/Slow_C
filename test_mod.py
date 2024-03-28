import subprocess

green = "\033[92m"
red = "\033[91m"
blue = "\033[94m"
cyan = "\033[96m"
end = "\033[0m"

color = blue

def compile_assert(name, src, expected, silent = False):
    if not silent:
        print(f"{cyan}--Testing {name}--{end}")
    f = open("tmp.txt", 'w')
    f.write(src)
    f.close()

    if expected == "False":
        expected = 0
    elif expected == "True":
        expected = 1


    import math
    expected = int(math.trunc(float(expected)))

    if silent:
        output = subprocess.DEVNULL
    else:
        output = None
    
    if not silent:
        print(color + "-------------- Run MyCompiler" + end)
    run_cmd = "main.exe tmp.txt".split()
    if subprocess.run(run_cmd, stdout=output).returncode != 0:
        print(f"{red}{name} didn't compile succesfully {end}")
        subprocess.run(run_cmd)
        return False


    if not silent:
        print(color + "-------------- Assemble out.asm" + end)
    assemble_cmd = "nasm -f win64 out.asm -o tmp\out.obj".split()
    if subprocess.run(assemble_cmd, stdout=output).returncode != 0:
        print(f"{red}{name} didn't assemble succesfully {end}")
        return False


    if not silent:
        print(color + "-------------- Link out.o" + end)
    link_cmd = "ld tmp\out.obj -o tmp\out.exe".split()
    if subprocess.run(link_cmd, stdout=output).returncode != 0:
        print(f"{red}{name} didn't link succesfully {end}")
        subprocess.run(link_cmd)
        return False


    if not silent:
        print(color + "-------------- Run out.exe" + end)
    rt = subprocess.run("tmp\out.exe".split(), stdout=output)
    
    if rt.returncode != expected:
        print(f"{red}Test {name} failed")
        print(f"Expeceted {expected}, got {rt.returncode}{end}")
        return False
    else:
        print(green + "Succesfully tested: " +  name + end)

    return True


## The first line is always the expected result
## the rest of the file is the source program
def test_file(file_name, silent = False):
    if os.path.exists(file_name):
        retrun;
    f = open(file_name)

    src = ""
    
    expected = int(f.readline())

    count = 1
    while True:
        count += 1
        line = f.readline()

        if not line:
            break

        src += line

    f.close()

    compile_assert(file_name, src, expected, silent)

def test_comp_operators():
    return (
        test_op('==', "equal") and 
        test_op('!=', "not equal") and
        test_op('>', "more than") and
        test_op('<', "less than") and
        test_op('>=', "more than or equal to") and
        test_op('<=', "less than or equal to") 
        )

def test_binops():
    test_op("+", "addition")
    test_op("-", "subtraction")
    test_op("*", "multiplication")
    test_op("/", "division")
    compile_assert("negative nums", "-1 - (-5)", 4, True)

def test_op(op, text):
    start = 1
    end = 10
    import random
    a = random.randint(start, end)
    b = random.randint(start, end)
    expr = f"{a} {op} {a}"
    return compile_assert(text, expr, str(eval(expr)), True)

