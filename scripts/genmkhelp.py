import sys
import re 
from colorama import init, Fore, Style

if len(sys.argv) != 2:
    print("ERROR: Invalid Args")
    sys.exit(1)

lines = []
with open(sys.argv[1], 'r') as mkfile:
    lines = mkfile.readlines()

header = []
variables = []
targets = []
footer = []

header_tag_re = re.compile(r'^.*#h#(.*)$')
variable_tag_re = re.compile(r'^.*#v#(.*)$')
variable_decl_re = re.compile(r'^\s*(.*)[?!:+]=(.*)$')
target_tag_re = re.compile(r'^^\s*(.*):.*#t#(.*)$')
footer_tag_re = re.compile(r'^.*#f#(.*)$')


var_tag = None
for l in lines:
    match = None

    # parse variable declaration, if last line had a var tag #v#
    if var_tag is not None:
        match = variable_decl_re.match(l)
        if match:
            variables += [[match.group(1).strip(), match.group(2).strip(), var_tag]]
        var_tag = None
        continue

    # Header tag: #h#
    match = header_tag_re.match(l)
    if match:
        header += [match.group(1).strip()]
        continue
    
    # Variable tag: #v#
    match = variable_tag_re.match(l)
    if match:
        var_tag = match.group(1).strip()
        continue

    match = target_tag_re.match(l)
    if match:
        targets += [[match.group(1).strip(), match.group(2).strip()]]
        continue

    # Footer
    match = footer_tag_re.match(l)
    if match:
        footer += [match.group(1).strip()]
        continue
    
# ==================== Print Help ====================
# Header
if len(header) > 0:
    print(f'{Fore.GREEN}'+'\n'.join(header)+f'{Fore.RESET}\n')

print('Usage: make', '[VARs]' if len(variables) > 0 else '', 'TARGETs' if len(targets) > 0 else '')

# variables
if len(variables) > 0:
    print('\nVARs:')
    for v in variables:
        print(Fore.CYAN+'\t{:<15s}{}{:<15s}'.format(v[0], Fore.RESET, v[2]), end='')
        print(f' (default: {v[1]})' if v[1] else '')

# targets
if len(targets) > 0:
    print('\nTARGETs:')
    for t in targets:
        print(Fore.CYAN+'\t{:<15s}{}{:<15s}'.format(t[0], Fore.RESET, t[1]))

# footer
if len(footer)>0:
    print(f'\n'+'\n'.join(footer))
