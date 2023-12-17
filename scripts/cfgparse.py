#!/usr/bin/python3

import json, os, sys, argparse

###############################################################################
# Helper functions

def throwerr(msg, code=1):
    """
    Throw error and exit
    """
    RED         = "\033[0;31m"
    RESET       = "\033[0m"
    print(f"{RED}CFGPARSE ERROR:{RESET} {msg}", file=sys.stderr)
    sys.exit(code)

def process_path(inp:any, passthrough=False)->any:
    """
    Resolve any environment variable given path(s)
    """
    if passthrough:
        return inp
    
    if isinstance(inp, str):
        return os.path.expanduser(os.path.expandvars(inp))
    elif isinstance(inp, list):
        for i in range(len(inp)):
            inp[i] = os.path.expanduser(os.path.expandvars(inp[i]))
        return inp

def remove_dups(lst: list):
    """
    Remove duplicate entries from list preserving the order
    """
    unique_list = [item for index, item in enumerate(lst) if item not in lst[:index]]
    return unique_list

def list2str(lst:list, one_per_line=False, line_terminator='\n'):
    return (line_terminator if one_per_line else ' ').join(lst)

def prefixlist(lst:list, prefix=''):
    return [prefix+x for x in lst]

def search_file(filename: str, search_dirs: list):
    for dir in search_dirs:
        fullpath = dir+'/'+filename
        if os.path.exists(fullpath):
            return fullpath
    return None

###############################################################################
# Supported Json config file structure (paths may include environment variables)
# module.json
# {
#     "name": "xyz",    // name of the module
#     "extends": "abc"  // derive configuration for xyz from abc overriding the params
#     "defines": [],    // preprocessor defines
#     "vsrcs": [],      // Verilog sources
#     "incdirs": []     // Verilog include directories
#     "includes" []     // config dependencies for current config
# }

class Config:
    def __init__(self, json_file:str) -> None:
        self.jsonfile = json_file
        self.json = {}
        self.deps = []

        # load json
        with open(json_file, 'r') as cf:
            self.json = json.load(cf)
        
        # check if json filename doesn't match name
        if os.path.basename(json_file).replace('.json', '') != self.get_name():
            throwerr(f'json file name, modulename mismatch for {self.jsonfile}')
        
    def get_name(self):
        return self.json['name'] if 'name' in self.json.keys() else '?'
    
    def get_params(self):
        return self.json['params'] if 'params' in self.json.keys() else dict()
    
    def get_type(self):
        return self.json['type'] if 'type' in self.json.keys() else 'generic'

    def get_vtopmodule(self):
        return self.eval_conditionals(self.json['vtopmodule']) if 'vtopmodule' in self.json.keys() else self.get_name()

    def get_vdefines(self)->list:
        return self.eval_conditionals(self.json['vdefines']) if 'vdefines' in self.json.keys() else list()
    
    def get_vsrcs(self)->list:
        return self.eval_conditionals(self.json['vsrcs']) if 'vsrcs' in self.json.keys() else list()

    def get_vincdirs(self)->list:
        return self.eval_conditionals(self.json['vincdirs']) if 'vincdirs' in self.json.keys() else list()

    def get_includes(self)->list:
        return self.eval_conditionals(self.json['includes']) if 'includes' in self.json.keys() else list()

    def get_attr(self, attr)->any:
        return self.eval_conditionals(self.json[attr]) if attr in self.json.keys() else None

    def eval_conditionals(self, ds:str or list or dict):
        """
            Evaluates non nested conditional statements in str, list, dict(not in key)
            fmt: [cond?A:B]
            cond can by a python style condition which can contain veriables 
            from the param field provided in the same json file. Depending on the condition 
            the whole conditonal block i.e. [?:] is replaced by either A or B
        """
        def __eval(txt:str):
            while '[' in txt:
                # get expr bounds
                expr_strt = txt.find('[')
                expr_end = txt.find(']', expr_strt)
                assert expr_end != -1, "malformed expression"

                # Get fields
                expr_que = txt.find('?', expr_strt)
                if expr_que == -1: # ? not found: just evaluate expr and emplace
                    condn = txt[expr_strt+1:expr_end]
                    res = str(eval(condn, self.get_params()))
                    txt = txt[0:expr_strt]+res+txt[expr_end+1:]
                    continue
                
                expr_col = txt.find(':', expr_que)
                if expr_strt == -1 or expr_que == -1 or expr_col == -1 or expr_end == -1:
                    raise f'Malformed expr in {self.jsonfile}:{txt}'
                condn = txt[expr_strt+1:expr_que]
                res = txt[expr_que+1:expr_col] if eval(condn, self.get_params()) else txt[expr_col+1:expr_end]
                txt = txt[0:expr_strt]+res+txt[expr_end+1:]                    
            return txt

        if isinstance(ds, str):
            ds = __eval(ds)
        elif isinstance(ds, list):
            tmp = []
            for l in ds:
                res = __eval(l)
                tmp += [res] if len(res.strip())>0 else [] # cleanup any blank entries
            ds = tmp
        elif isinstance(ds, dict):
            for k in ds.keys():
                ds[k] = __eval(ds[k])
        else:
            raise f'cannot resolve expession in type {type(ds)}'
        return ds



class ConfigParser:
    def __init__(self, cfg:Config, disable_resolve_env=True) -> None:
        assert isinstance(cfg, Config), f'Unknown type for config {type(cfg)}'
        self.cfg = cfg
        self.search_dirs = []
        self.disable_resolve_env = disable_resolve_env
    
    def add_searchdir(self, dir: str):
        dir = process_path(dir)
        if dir not in self.search_dirs:
            self.search_dirs = [dir] + self.search_dirs

    def parse(self, override_params=[]):
        # check if extends
        def chk_extends(cfg:Config):
            if 'extends' in cfg.json.keys():
                import copy
                bak = copy.deepcopy(cfg)
                parent_cfg = cfg.json['extends']
                parent_cfg_file = search_file(parent_cfg+'.json', self.search_dirs)
                assert parent_cfg_file is not None, f'Cannot find parent "{parent_cfg}" for "{cfg.get_name()}"\n\tSearched in {str(self.search_dirs)}'
                with open(parent_cfg_file, 'r') as cf:
                    cfg.json = json.load(cf)
                # override name
                cfg.json['name'] = bak.get_name()
                # override params
                for param in bak.get_params().keys():
                    cfg.json['params'][param] = bak.get_params()[param] # updates, adds if not presents
            for dep in cfg.deps:
                chk_extends(dep)
        chk_extends(self.cfg)

        # override params
        if len(override_params) > 0:
            for param in override_params:
                pname, pvalue = param.split('=')
                if pname in self.cfg.json['params']:
                    self.cfg.json['params'][pname] = pvalue

        def resolve_config_deps(cfg: Config):
            # resolve deps in current config
            for inc_cfg_name in cfg.get_includes():
                assert inc_cfg_name != cfg.get_name(), f'Cannot resolve circular dependency: "{inc_cfg_name}" in {cfg.get_name()}'                
                inc_cfg_file = search_file(inc_cfg_name+'.json', self.search_dirs)
                assert inc_cfg_file is not None, f'Cannot find dependency "{inc_cfg_name}" for "{cfg.get_name()}"\n\tSearched in {str(self.search_dirs)}'
                
                cfg.deps += [Config(inc_cfg_file)]

            # resolve deps of in current config's deps
            for cfg_dep in cfg.deps:
                resolve_config_deps(cfg_dep)

        # Resolve dependencies recursively
        resolve_config_deps(self.cfg)

    def get_hierarcy(self):
        def print_cfg(cfg:Config, tab, ntabs=0):
            txt = f'{tab * ntabs}- {cfg.get_name()}\n'
            for inc_cfg in cfg.deps:
                txt += print_cfg(inc_cfg, tab, ntabs+1)
            return txt
        return print_cfg(self.cfg, '   ')

    def get_name(self):
        return self.cfg.get_name()
    
    def get_type(self):
        return self.cfg.get_type()

    def get_verilog_topmodule(self):
        return self.cfg.get_vtopmodule()
    
    def get_verilog_defines(self)->list:
        def get_vdefines_recursively(cfg:Config):
            vdefs = list()
            vdefs += cfg.get_vdefines()
            for dep in cfg.deps:
                vdefs += get_vdefines_recursively(dep)
            return vdefs
        return process_path(remove_dups(get_vdefines_recursively(self.cfg)), self.disable_resolve_env)
    
    def get_verilog_srcs(self)->list:
        def get_vsrcs_recursively(cfg:Config):
            vsrcs = list()
            vsrcs += cfg.get_vsrcs()
            for dep in cfg.deps:
                vsrcs += get_vsrcs_recursively(dep)
            return vsrcs
        return process_path(remove_dups(get_vsrcs_recursively(self.cfg)), self.disable_resolve_env)

    def get_verilog_incdirs(self)->list:
        def get_vincdirs_recursively(cfg:Config):
            vincdirs = list()
            vincdirs += cfg.get_vincdirs()
            for dep in cfg.deps:
                vincdirs += get_vincdirs_recursively(dep)
            return vincdirs
        return process_path(remove_dups(get_vincdirs_recursively(self.cfg)), self.disable_resolve_env)

    def gen_tcl(self, tool='yosys')->str:
        txt  =  '#'*80+'\n'
        txt += f'# Autogenerated TCL for "{self.cfg.get_name()}"\n'
        txt +=  '#'*80+'\n\n'

        txt += 'set defines [list \\\n'
        for dfn in self.get_verilog_defines():
            txt += f'\t"{dfn}" \\\n'
        txt += ']\n\n'

        txt += 'set include_dirs [list \\\n'
        for idir in self.get_verilog_incdirs():
            txt += f'\t"{idir}" \\\n'
        txt += ']\n\n'

        txt += 'set design_sources [list \\\n'
        for vsrc in self.get_verilog_srcs():
            txt += f'\t"{vsrc}" \\\n'
        txt += ']\n\n'

        txt += f'set topmodule {self.get_verilog_topmodule()}\n'
        return txt
    
    def gen_filelist(self, tool='verilator'):
        txt  =  '//'*40+'\n'
        txt += f'// Autogenerated File list for "{self.get_name()}"\n'
        txt +=  '//'*40+'\n\n'

        for dfn in self.get_verilog_defines():
            txt += f'-D{dfn}\n'

        for idir in self.get_verilog_incdirs():
            txt += f'+incdir+{idir}\n'

        if tool == 'verilator':
            txt += f'--set-topmodule {self.get_verilog_topmodule()}\n'
        elif tool == 'iverilog':
            txt += f'-s {self.get_verilog_topmodule()}\n'
        else:
            raise f'Unsupported tool "{tool}"'
        
        for vsrc in self.get_verilog_srcs():
            txt += f'{vsrc}\n'
        return txt

    def get_vflags(self, tool='verilator') -> list:
        flags = list()
        flags += prefixlist(self.get_verilog_defines(), '-D')
        flags += prefixlist(self.get_verilog_incdirs(), '-I')
        vtop = self.get_verilog_topmodule()
        if tool == 'verilator':
            flags += [f'--top-module {vtop}']
        elif tool == 'iverilog':
            flags += [f'-s {vtop}']
        else:
            raise f'Unsupported tool "{tool}"'
        flags += self.get_verilog_srcs()
        return flags


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Parses RISCV-Atom JSON config files and dumps requested info to stdout / file')
    parser.add_argument('json', type=str, help='JSON config file')
    parser.add_argument('-l', '--one-per-line', help='Print one entry per line', action='store_true')
    parser.add_argument('-o', '--output', help='Dump output to file', type=str)
    parser.add_argument('-n', '--no-resolve', help='Do not resolve environment variables in path', action='store_true')
    parser.add_argument('-T', '--tool', help='Specify tool', type=str, default='generic')
    parser.add_argument('-p', '--param', action='append', help='override a parameter', type=str)

    mutex_group = parser.add_mutually_exclusive_group()
    mutex_group.add_argument('-t', '--top', help='get verilog topmodule', action='store_true')
    mutex_group.add_argument('-i', '--incdirs', help='get verilog include directories', action='store_true')
    mutex_group.add_argument('-v', '--vsrcs', help='get verilog sources', action='store_true')
    mutex_group.add_argument('-d', '--defines', help='get verilog defines', action='store_true')
    mutex_group.add_argument('-f', '--vflags', help='get verilog sim flags', action='store_true')
    mutex_group.add_argument('-c', '--tcl', help='generate tcl script', action='store_true')
    mutex_group.add_argument('-F', '--filelist', help='generate filelist', action='store_true')
    mutex_group.add_argument('-a', '--get-attr', help='get raw attribute', type=str)
    mutex_group.add_argument('-H', '--hierarchy', help='print module hierarchy', action='store_true')

    # Parse args
    args = parser.parse_args()

    try:
        # Create Config from json
        cfg = Config(args.json)
        cfgp = ConfigParser(cfg, args.no_resolve)
        
        # add default search path
        rvatom_root = os.getenv('RVATOM')
        if rvatom_root is None:
            throwerr('"RVATOM" environment variable not defined, did you forget to source sourceme file?')
        else:
            cfgp.add_searchdir(rvatom_root+'/rtl/config')       
        
        # parse
        cfgp.parse(override_params=args.param if args.param else [])
        
        # obtain
        txt = ''
        if args.top:
            txt += cfgp.get_verilog_topmodule()

        if args.incdirs:
            txt = list2str(cfgp.get_verilog_incdirs(), args.one_per_line)

        if args.vsrcs:
            txt = list2str(cfgp.get_verilog_srcs(), args.one_per_line)

        if args.defines:
            txt = list2str(cfgp.get_verilog_defines(), args.one_per_line) 

        if args.vflags:
            txt = list2str(cfgp.get_vflags('verilator' if args.tool == 'generic' else args.tool))
        
        if args.tcl:
            txt = cfgp.gen_tcl('verilator' if args.tool == 'generic' else args.tool)

        if args.filelist:
            txt = cfgp.gen_filelist('verilator' if args.tool == 'generic' else args.tool)
        
        if args.hierarchy:
            txt = cfgp.get_hierarcy()
        
        if args.get_attr:
            txt = str(cfgp.cfg.get_attr(args.get_attr))

        # Dump requested info
        if args.output:
            with open(args.output, 'w') as f:
                f.write(txt)
        else:
            print(txt)
    
    except AssertionError as e:
        throwerr(e)
    except FileNotFoundError as e:
        throwerr(e)
    except Exception as e:
        raise e
