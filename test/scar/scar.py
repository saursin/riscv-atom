#!/usr/bin/python3
################################################################################
#                      RISC-V Atom verification framework                      #
################################################################################
import os, sys, enum
import re, datetime


class Color:
    RED         = "\033[0;31m"
    GREEN       = "\033[0;32m"
    YELLOW      = "\033[1;33m"
    CYAN        = "\033[0;36m"
    PURPLE      = "\033[0;35m"

    BOLD        = "\033[1m"
    CROSSED     = "\033[9m"
    FAINT       = "\033[2m"
    ITALIC      = "\033[3m"
    UNDERLINE   = "\033[4m"
    RESET       = "\033[0m"

    def disable_colors():
        for a in dir(Color):
            if isinstance(a, str) and a[0] != "_":
                setattr(Color, a, '')


RISCV_REG_ALIASES = {
	"zero"  : "x0",
	"ra"    : "x1",
	"sp"    : "x2",
	"gp"    : "x3",
	"tp"    : "x4",
	"t0"    : "x5",
	"t1"    : "x6",
	"t2"    : "x7",
	"s0"    : "x8",
    "fp"    : "x8",
	"s1"    : "x9",
	"a0"    : "x10",
	"a1"    : "x11",
	"a2"    : "x12",
	"a3"    : "x13",
	"a4"    : "x14",
	"a5"    : "x15",
	"a6"    : "x16",
	"a7"    : "x17",
	"s2"    : "x18",
	"s3"    : "x19",
	"s4"    : "x20",
	"s5"    : "x21",
	"s6"    : "x22",
	"s7"    : "x23",
	"s8"    : "x24",
	"s9"    : "x25",
	"s10"   : "x26",
	"s11"   : "x27",
	"t3"    : "x28",
	"t4"    : "x29",
	"t5"    : "x30",
	"t6"    : "x31"
}


class ReturnCodes(enum.Enum):
    COMPILE_ERR=0
    COMPILE_SUCCESS=1

    EXEC_ERR=2
    EXEC_SUCCESS=3

    VERIF_ASSERTION_FILE_DOES_NOT_EXIST=4
    VERIF_NO_ASSERTION=5
    VERIF_SOME_ASSERTIONS_FAILED=6
    VERIF_SOME_ASSERTIONS_IGNORED=7
    VERIF_ALL_ASSERTIONS_PASSED=8




def run_cmd(cmd:list, print_dumps=True):
    import subprocess
    dump = subprocess.run(cmd, capture_output=True, text=True)
    if len(dump.stdout) != 0 and print_dumps:
        print('stdout:\n' + dump.stdout)
    if len(dump.stderr) != 0 and print_dumps:
        print('stderr:\n'+ dump.stderr)
    return dump




def compile_test(test:dict, save_objdump:bool=False):
    # ---------- Configuration ----------
    RVPREFIX = 'riscv64-unknown-elf-'
    CC = 'gcc'
    CFLAGS = ['-march=rv32im', '-mabi=ilp32', '-nostartfiles']
    LDFLAGS = []
    # select linkerscript by auto detecting soctarget
    try:
        dump = run_cmd(['atomsim', '--soctarget'], print_dumps=False)
        soctarget = dump.stdout.replace('\n', '')
        LDFLAGS = ['-T', os.getenv('RVATOM')+'/sw/lib/link/link_'+soctarget+'.ld']
    except Exception as e:
        print(e)
        print('Failed to get soctarget')
        sys.exit(1)
    
    ELF_FILE = WORKDIR+f'/{test["name"]}.elf'
    OBJDUMP_FILE = WORKDIR+f'/{test["name"]}.lst'
    # -----------------------------------
    
    # Compile
    compile_cmd = [RVPREFIX+CC] + CFLAGS + test["srcs"] + ['-o', ELF_FILE] + LDFLAGS
    if VERBOSE:
        print(" ".join(compile_cmd))

    dump = run_cmd(compile_cmd, print_dumps=VERBOSE)
    if dump.returncode != 0:
        print(Color.RED+"Compilation error!"+Color.RESET)
        return ReturnCodes.COMPILE_ERR, None

    # Generate Objdump
    if save_objdump:
        dump = run_cmd([RVPREFIX+'objdump', '-Shtd', ELF_FILE], print_dumps=False)
        with open(OBJDUMP_FILE, 'w') as f:
            f.write(dump.stdout)

    return ReturnCodes.COMPILE_SUCCESS, {"elf_file": ELF_FILE, "objdump_file": OBJDUMP_FILE}




def execute_test(test:dict, compile_outputs:dict, mute=True):
    # ---------- Configuration ----------
    VCD_FILE = WORKDIR+f'/{test["name"]}.vcd'
    DUMP_FILE = WORKDIR+f'/{test["name"]}.dump'

    EXEC = 'atomsim'
    EXEC_FLAGS = ['--ebreak-dump', '--no-banner', '--maxitr', '100000', '--trace-file', VCD_FILE, '--dump-file', DUMP_FILE, '-v']
    # -----------------------------------

    # Execute test
    exec_cmd = [EXEC]+EXEC_FLAGS+[compile_outputs["elf_file"]]
    if VERBOSE:
        print(" ".join(exec_cmd))
    
    dump = run_cmd(exec_cmd, print_dumps=VERBOSE)
    if (dump.returncode != 0):
        print(Color.RED+"Execution error!"+Color.RESET)
        return ReturnCodes.EXEC_ERR, None
    elif len(dump.stderr) != 0:
        return ReturnCodes.EXEC_ERR, None
    else:
        return ReturnCodes.EXEC_SUCCESS, {"vcd_file": VCD_FILE, "dump_file": DUMP_FILE}




def verify_test(test:dict, compile_outputs:dict, execute_outputs:dict):    
    # get dump contents
    reg_vals={}
    with open(execute_outputs["dump_file"], 'r') as dumpfile:
        for line in dumpfile:
            tokens = line.strip().split(' ')
            reg_vals[tokens[0]] = int(tokens[1], 16)
    

    # get assertion file contents
    def remove_comments_from_line(line):
        if '#' in line:
            line = line.split('#', 1)[0]
        return line.strip()
            
    assertions = []
    if not os.path.exists(test["assertion_file"]):
        return ReturnCodes.VERIF_ASSERTION_FILE_DOES_NOT_EXIST
    
    with open(test["assertion_file"], 'r') as assertionfile:
        for line in assertionfile:
            line = remove_comments_from_line(line)
            if line != '':
                assertions += [line.strip()]
    

    # evaluate assertions
    if len(assertions) == 0:
        return ReturnCodes.VERIF_NO_ASSERTION

    def resolve_aliases(input_string, alias_mapping):
        variable_names = re.findall(r'\b\w+\d*\b', input_string)
        modified_string = input_string
        for var_name in variable_names:
            if var_name in alias_mapping:
                modified_string = re.sub(r'\b' + var_name + r'\b', str(alias_mapping[var_name]), modified_string)
        return modified_string

    results = []
    for i, raw_expr in enumerate(assertions):
        expr = resolve_aliases(raw_expr, RISCV_REG_ALIASES)
        res = None
        try:
            res = eval(expr, reg_vals) == True
        except Exception as e:
            print(f"Error:{test['assertion_file']}:{i+1}", f'"{raw_expr}"', e)
            res = None
        finally:
            Res = {
                True: Color.GREEN+'PASS'+Color.RESET,
                False: Color.RED+'FAIL'+Color.RESET,
                None: Color.YELLOW+'IGNORED'+Color.RESET
            }
            print('assert:', raw_expr, ':',  Res[res])
        
        results += [res]
    
    print('')
    if False in results:
        print(Color.RED+"Some Assertions Failed"+Color.RESET)
        return ReturnCodes.VERIF_SOME_ASSERTIONS_FAILED
    elif None in results:
        print(Color.YELLOW+"Some Assertions Ignored"+Color.RESET)
        return ReturnCodes.VERIF_SOME_ASSERTIONS_IGNORED
    else:
        print(Color.GREEN+"All Assertions Passed! "+Color.RESET)
        return ReturnCodes.VERIF_ALL_ASSERTIONS_PASSED



if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('-v', '--verbose', help='Enable verbose output', action='store_true')
    parser.add_argument('-w', '--workdir', help='Specify work directory', type=str, default='work')
    parser.add_argument('-o', '--output', help='Specify report output file', type=str, default='work/scartest.report')
    parser.add_argument('--nocolor', help='Disable colors', action='store_true')

    parser.add_argument('json', help='provide a json file containing tests list', type=str)
    args = parser.parse_args()

    global VERBOSE, WORKDIR
    VERBOSE = args.verbose
    WORKDIR = args.workdir

    if args.nocolor or not sys.stdout.isatty():
        Color.disable_colors()

    print(Color.YELLOW, end="")
    print("|==============================================|")
    print("|      RISCV-Atom Verification framework       |")
    print("|==============================================|")
    print("  By: Saurabh Singh(saurabh.s99100@gmail.com)")
    print(Color.RESET)

    # Get test list
    if(VERBOSE):
        print(Color.CYAN+f"> Getting tests from: {args.json}"+Color.RESET)
    
    tests = None
    with open(args.json, 'r') as jf:
        import json
        tests = json.load(jf)

    if (VERBOSE):
        print("Found ", len(tests), "tests!")
        for i, t in enumerate(tests):
            print(" {: <6s}{:s}".format(str(i)+':', t["name"]))
        print(80*"=")

    
    test_db = []
    for i, test in enumerate(tests):
        print(Color.CYAN+"Test: "+Color.RESET+test["name"]+'\n')
        # Compile test
        print(Color.PURPLE + 'Compiling..' + Color.RESET)
        compile_rc, compile_outputs = compile_test(test, save_objdump=True)

        # Execute test
        print(Color.PURPLE + 'Executing..' + Color.RESET)
        execute_rc, execute_outputs = execute_test(test, compile_outputs)

        # Verify test
        print(Color.PURPLE + 'Verifying..' + Color.RESET)
        verify_rc = verify_test(test, compile_outputs, execute_outputs)

        test_db += [{
            "name": test["name"],
            "compile_rc": compile_rc,
            "execute_rc": execute_rc,
            "verify_rc": verify_rc
        }]
        
        if i != len(tests)-1: 
            print('-'*80)

    print('='*80)

    # Generate report
    print(Color.CYAN+"> Generating report:"+Color.RESET+f' {args.output}')
    rpt_txt =  '+'+'-'*78+'+\n'
    rpt_txt += '|{: ^78s}|\n'.format('SCAR Verification Report')
    rpt_txt += '+'+'-'*78+'+\n'
    curr_datetime = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    rpt_txt +=f'Date: {curr_datetime}\n\n'

    ignored_tests = []
    failed_tests = []
    passed_tests = []
    for i, test in enumerate(test_db):
        test_status = '?'
        reason = ''
        if test["compile_rc"] == ReturnCodes.COMPILE_ERR:
            reason = 'Compile Error'
            test_status = f'{Color.YELLOW}Ignored{Color.RESET}'
            ignored_tests += [test["name"]]
        elif test["compile_rc"] == ReturnCodes.COMPILE_SUCCESS:
            if test["execute_rc"] == ReturnCodes.EXEC_ERR:
                reason = 'Execute Error'
                test_status = f'{Color.YELLOW}Ignored{Color.RESET}'
                ignored_tests += [test["name"]]
            elif test["execute_rc"] == ReturnCodes.EXEC_SUCCESS:
                if test["verify_rc"] == ReturnCodes.VERIF_ASSERTION_FILE_DOES_NOT_EXIST:
                    reason = 'Assertion File Doesn\'t Exist'
                    test_status = f'{Color.YELLOW}Ignored{Color.RESET}'
                    ignored_tests += [test["name"]]
                elif test["verify_rc"] == ReturnCodes.VERIF_NO_ASSERTION:
                    reason = 'No Assertions'
                    test_status = f'{Color.YELLOW}Ignored{Color.RESET}'
                    ignored_tests += [test["name"]]
                elif test["verify_rc"] == ReturnCodes.VERIF_SOME_ASSERTIONS_FAILED:
                    reason = 'Some Assertions Failed'
                    test_status = f'{Color.RED}Failed{Color.RESET}'
                    failed_tests += [test["name"]]          
                elif test["verify_rc"] == ReturnCodes.VERIF_SOME_ASSERTIONS_IGNORED:
                    reason = 'Some Assertions Ignored'
                    test_status = f'{Color.RED}Failed{Color.RESET}'
                    failed_tests += [test["name"]]
                elif test["verify_rc"] == ReturnCodes.VERIF_ALL_ASSERTIONS_PASSED:
                    reason = 'All Assertions Passed'
                    test_status = f'{Color.GREEN}Passed{Color.RESET}'
                    passed_tests += [test["name"]]
                else:
                    print('Internal Err: Invalid verif_test() return code:', test["verify_rc"])
                    sys.exit(1)             
            else:
                print('Internal Err: Invalid execute_test() return code:', test["execute_rc"])
                sys.exit(1)
        else:
            print('Internal Err: Invalid compile_test() return code:', test["compile_rc"])
            sys.exit(1)

        rpt_txt += '{: <7s} {: <20s} - {: <18s}  {: <20s} \n'.format(str(i)+').', test["name"], test_status, Color.FAINT+reason+Color.RESET)

    rpt_txt += '='*80 + '\n'

    n_failed_tests = len(failed_tests)
    n_ignored_tests = len(ignored_tests)
    n_passed_tests = len(passed_tests)
    n_total_tests = len(test_db)

    rpt_txt += Color.GREEN + f"Passed tests  : {n_passed_tests} / {n_total_tests}" + Color.RESET + '\n'
    rpt_txt += Color.YELLOW + f"Ignored tests : {n_ignored_tests} / {n_total_tests}" + Color.RESET + '\n'
    rpt_txt += Color.RED + f"Failed tests  : {n_failed_tests} / {n_total_tests}" + Color.RESET + '\n'

    print(rpt_txt)

    with open(args.output, 'w') as of:
        of.write(rpt_txt)

    sys.exit(1 if n_failed_tests > 0 or n_ignored_tests > 0 else 0)
