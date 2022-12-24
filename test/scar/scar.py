#########################################
#  RISCV - Atom verification framework  #
#########################################
import os
import subprocess
import glob
from colorama import Fore, Back, Style


linker_script_path = 'link.ld' #relative
cwd = os.getcwd()
work_dir = cwd + '/work'


RVPREFIX = 'riscv64-unknown-elf-'
CC = 'gcc'
CFLAGS = ['-march=rv32i', '-mabi=ilp32', '-nostartfiles']
LDFLAGS = ['-T', linker_script_path]

EXEC = 'atomsim'
EXEC_FLAGS = ['--ebreak-dump', '--maxitr', '1000', '--trace-file', work_dir+'/trace.vcd', '--dump-file', work_dir+'/dump.txt', '-v']


###############################################################################################
reg_names = {
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


def search():
    # get all tests
    return sorted(glob.glob('*.S'), key=str.lower)





def compile(tests, save_objdump=False):
    for t in tests:
        print(Fore.GREEN+'compile: '+Style.RESET_ALL+t)
        dump = subprocess.run(
            [RVPREFIX+CC]+CFLAGS+LDFLAGS+[t, '-o', work_dir+'/'+t[0:-2]+'.elf'] , capture_output=True, text=True
        )
        if len(dump.stdout) != 0:
            print('stdout: ' + dump.stdout)
        if len(dump.stderr) != 0:
            print('stderr: '+ dump.stderr)

        if (dump.returncode != 0):
            print(Fore.RED+"EXITING due to compile error!"+Style.RESET_ALL)
            exit()

        # get objdumpdump
        if save_objdump:
            dump = subprocess.run(
                [RVPREFIX+'objdump', '-htd', work_dir+'/'+t[0:-2]+'.elf'], capture_output=True, text=True
            )
            if len(dump.stdout) != 0:
                with open(work_dir+'/'+t[0:-2]+'.objdump', 'w') as f:
                    f.write(dump.stdout)
            if len(dump.stderr) != 0:
                print('stderr: '+ dump.stderr)





def execute(test, mute=True):
    print(40*"-")
    elf = work_dir+'/'+test[0:-2]+'.elf'
    print(Fore.GREEN+'execute: '+Style.RESET_ALL + elf)

    if not mute:
        for item in [EXEC]+EXEC_FLAGS+[elf]:
            print(item, end=" ")
        print("")

    dump = subprocess.run(
        [EXEC]+EXEC_FLAGS+[elf] , capture_output=True, text=True
    )
    if not mute and len(dump.stdout) != 0:
        print('stdout: ' + dump.stdout)
    if len(dump.stderr) != 0:
        print('stderr: '+ dump.stderr)

    if (dump.returncode != 0):
        print(Fore.RED+"Execution error!"+Style.RESET_ALL)
        return False
    elif len(dump.stderr)!=0:
        return False
    else:
        return True





def verify(test):
    print(Fore.GREEN+'verify: '+Style.RESET_ALL + test)
    fcontents = []

    # read assembly files
    try:
        f = open(test, 'r')
        fcontents = f.readlines()
        f.close()
    except:
        print("Unable to open file :" + test)
        exit()

    # find start of assert block
    assert_block_start = -1
    for i in range(0, len(fcontents)):
        if fcontents[i].strip() == "# $-ASSERTIONS-$":
            assert_block_start = i
            break
    
    # if no assertions found
    if assert_block_start == -1:
        print(Fore.YELLOW+"Skipping: no assertions!"+Style.RESET_ALL)
        return None

    
    # copy dump file   
    os.system('cp '+work_dir+'/dump.txt '+work_dir+'/'+test[0:-2]+'_dump.txt')


    # get dump data
    dump_file = work_dir+'/'+'dump.txt'
    dcontents = []
    with open(dump_file, 'r') as f:
        dcontents = f.readlines()
    
    dump_data = {}
    # create a dictionary of this data
    for l in dcontents:
        if l.strip() == "":
            continue

        l = l.strip().split(' ')
        dump_data[l[0]] = l[1]
    
    # process & check assertions
    assertions = fcontents[assert_block_start+1:len(fcontents)]

    passed = [False]*len(assertions)
    for k, assr in enumerate(assertions):
        # Parse assertions one-by-one
        if assr.strip() == "":
            continue

        assr = assr[2:].strip()
        assr = assr.split(" ")
        
        assr_op = assr[0]
        assr_rg = assr[1]
        assr_val = assr[2]
        
        for dump_rg in dump_data.keys():
            # convert dumpfile regname if needed
            if dump_rg[0] == 'x':       # pure name
                pass
            else:
                if dump_rg in ['pc', 'ir']:
                    pass
                elif dump_rg in reg_names.keys():
                    dump_rg = reg_names[dump_rg] # convert to pure name if abi name    
                else:
                    print(f"Error parsing reg name \"{dump_rg}\" in dump file \"{dump_file}\"")

            # check
            if dump_rg != assr_rg:
                continue

            if assr_op == 'eq' and assr_val != dump_data[dump_rg]:
                print(Fore.RED+"Assertion Failed: "+Style.RESET_ALL+f"(expected {assr_val}, got: {dump_data[dump_rg]})")
                passed[k] = False
            else:
                passed[k] = True
    
    if False in passed:
        print(Fore.RED+"Some Assertions Failed"+Style.RESET_ALL)
        return False
    else:
        print(Fore.GREEN+"All Assertions Passed! "+Style.RESET_ALL)
        return True







if __name__ == "__main__":
    print(Fore.YELLOW, end="")
    print("|==============================================|")
    print("|      RISCV-Atom Verification framework       |")
    print("|==============================================|")
    print("  By: Saurabh Singh(saurabh.s99100@gmail.com)")


    # Search tests
    print(Fore.CYAN+"> Stage-1:"+Style.RESET_ALL+" Seaching for tests...")
    tests = search()
    print("Found "+str(len(tests))+' tests')
    for t in tests:
        print(t)
    
    print(80*"=")

    # Compile all
    print(Fore.CYAN+"> Stage-2:"+Style.RESET_ALL+" Compiling tests...")
    compile(tests, save_objdump=True)

    print(80*"=")

    # Execute one by one
    print(Fore.CYAN+"> Stage-3:"+Style.RESET_ALL+" Executing & verifying dumps...")
    
    failed_tests = []
    ignored_tests = []
    passed_tests = []

    for t in tests:
        execute_status = execute(t, mute=False)
        verify_status = verify(t)

        if execute_status == True:
            if verify_status == True:
                passed_tests = passed_tests + [t]
            elif verify_status == False:
                failed_tests = failed_tests + [t]
            elif verify_status == None:
                ignored_tests = ignored_tests + [t]
            else:
                print("Unknown return Value from execute function")
        else:
            ignored_tests = ignored_tests + [t]


    print(80*"=")

    # Conclude
    print(Fore.CYAN+"> Stage-4:"+Style.RESET_ALL+" Generating report...")
    print("|----------------------------|")
    print("|    Verification Report     |")
    print("|----------------------------|")

    print("Total tests  : " + str(len(tests)))

    count = 1
    for t in tests:
        print(str(count)+").\t"+t, end="")

        print(" "*(30-len(t)), end="- ")

        if t in passed_tests:
            print(Fore.GREEN+"Passed"+Style.RESET_ALL)
        elif t in ignored_tests:
            print(Fore.YELLOW+"Ignored"+Style.RESET_ALL)
        else:
            print(Fore.RED+"Failed"+Style.RESET_ALL)
        
        count=count+1

    
    print(Fore.GREEN+"\nPassed tests  : " + str(len(passed_tests)) + '/' + str(len(tests))+ Style.RESET_ALL)
    print(Fore.YELLOW+"Ignored tests : " + str(len(ignored_tests)) + '/' + str(len(tests))+ Style.RESET_ALL)
    print(Fore.RED+"Failed tests  : " + str(len(failed_tests)) + '/' +str(len(tests))+ Style.RESET_ALL)