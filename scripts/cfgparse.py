#!/usr/bin/env python3
"""YAML-based config parser and filelist generator for RISC-V Atom.

Template syntax used in YAML string values:
  $[param]                    -- value of param (dot-path for nested: a.b.c), or '' if undefined
  $[env.VAR]                  -- environment variable or ''
  $[shell CMD]                -- stdout of a shell command
  $[expr?true_val:false_val]  -- ternary; expr is eval'd with params in scope
  $[param?:false_val]         -- short form: '' when true, false_val when false
  $[p?VALUE\\?\\:END:]        -- escape literal ? or : inside a branch with \\? \\:
  Branches may contain nested $[...] expressions.
"""

import os
import sys
import subprocess
from pathlib import Path

import yaml


def evaluate_conditionals(txt, params, strict=True):
    """Expand all $[...] tokens in txt using params dict.

    All parsing helpers live here to avoid polluting the module namespace.
    """

    def closing(s, open_at):
        """Index of ] that closes [ at open_at, tracking nested $[...]."""
        depth, i = 1, open_at + 1
        while i < len(s):
            if s[i] == '$' and i + 1 < len(s) and s[i + 1] == '[':
                depth += 1; i += 2
            elif s[i] == ']':
                depth -= 1
                if depth == 0:
                    return i
                i += 1
            else:
                i += 1
        return -1

    def split_ternary(s):
        """Split 'expr?true_val:false_val' at outermost unescaped ? and :.
        Returns (expr, true_val, false_val) or None when no ? is present."""
        depth, q, c, i = 0, None, None, 0
        while i < len(s):
            ch = s[i]
            if ch == '\\' and i + 1 < len(s) and s[i + 1] in '?:':
                i += 2; continue
            if ch == '$' and i + 1 < len(s) and s[i + 1] == '[':
                depth += 1; i += 2; continue
            if ch == ']' and depth:
                depth -= 1; i += 1; continue
            if not depth:
                if   ch == '?' and q is None:                    q = i
                elif ch == ':' and q is not None and c is None:  c = i
            i += 1
        if q is None:
            return None
        return s[:q], (s[q+1:c] if c is not None else s[q+1:]), (s[c+1:] if c is not None else '')

    class _NS:
        """Wrap a nested dict so dotted attribute access works inside eval()."""
        def __init__(self, d):
            for k, v in d.items():
                setattr(self, k, _NS(v) if isinstance(v, dict) else v)
        def __bool__(self):
            return bool(vars(self))

    def _ns_dict(d):
        return {k: _NS(v) if isinstance(v, dict) else v for k, v in d.items()}

    def _getpath(key):
        """Traverse a dotted key path through the params dict."""
        cur = params
        for part in key.split('.'):
            if not isinstance(cur, dict) or part not in cur:
                return None, False
            cur = cur[part]
        return cur, True

    def eval_bool(expr):
        try:
            return bool(eval(expr, {"__builtins__": {}}, _ns_dict(params)))
        except (NameError, AttributeError) as e:
            if strict:
                raise KeyError(f"Undefined parameter in expression '{expr}': {e}") from e
            return False
        except Exception as e:
            raise ValueError(f"Error evaluating '{expr}': {e}") from e

    def expand(inner):
        if inner.startswith('shell '):
            return subprocess.check_output(inner[6:], shell=True, text=True).strip()
        if inner.startswith('env.'):
            return os.environ.get(inner[4:], '')
        parts = split_ternary(inner)
        if parts:
            expr, true_val, false_val = parts
            chosen = true_val if eval_bool(expr.strip()) else false_val
            return evaluate_conditionals(chosen.replace('\\?', '?').replace('\\:', ':'), params, strict)
        key = inner.strip()
        v, found = _getpath(key)
        if not found:
            if strict:
                raise KeyError(f"Undefined parameter: '{key}'")
            return ''
        return str(v) if v is not None else ''

    if not isinstance(txt, str):
        return txt
    out, i = [], 0
    while i < len(txt):
        if txt[i] == '$' and i + 1 < len(txt) and txt[i + 1] == '[':
            close = closing(txt, i + 1)
            if close == -1:
                out.append(txt[i]); i += 1; continue
            out.append(expand(txt[i + 2:close]))
            i = close + 1
        else:
            out.append(txt[i]); i += 1
    return ''.join(out)


class Config:
    def __init__(self):
        self.cfg = {}
        self.params = {}
        self.include_paths = []
        self.files = []     # every YAML file loaded (top-level + imports)
        self.strict = True  # raise on undefined params; disable with --allow-undefined

    # ---- loading / saving --------------------------------------------------

    def from_file(self, yml_file):
        """Load a YAML file and register its directory for import resolution."""
        p = Path(yml_file).resolve()
        with open(p) as f:
            self.cfg = yaml.safe_load(f) or {}
        self.files.append(p)
        self._add_inc(p.parent)

    def to_file(self, out=None):
        """Write resolved config as YAML to a path, file object, or stdout."""
        txt = yaml.dump(self.cfg, default_flow_style=False, sort_keys=False)
        if out is None or out is sys.stdout:
            sys.stdout.write(txt)
        elif isinstance(out, (str, Path)):
            Path(out).write_text(txt)
        else:
            out.write(txt)

    def set(self, dotted_key, value):
        """Override a cfg key using dot-separated path before parsing."""
        keys = dotted_key.split('.')
        d = self.cfg
        for k in keys[:-1]:
            d = d.setdefault(k, {})
        try:
            d[keys[-1]] = yaml.safe_load(value)
        except Exception:
            d[keys[-1]] = value

    def get(self, dotted_key):
        """Return a cfg value by dot-separated path. Mirror of set();
        raises KeyError if the path is absent."""
        val = self.cfg
        for k in dotted_key.split('.'):
            if not isinstance(val, dict) or k not in val:
                raise KeyError(dotted_key)
            val = val[k]
        return val

    # ---- parsing -----------------------------------------------------------
    def parse(self):
        """
        1. Resolve import chains and deep-merge into a single flat config.
        2. Evaluate derived params in declaration order.
        3. Substitute $[...] in all remaining string fields.
        """
        self.cfg    = self._resolve_imports(self.cfg)
        self.params = self._eval_params(self.cfg.get('params', {}))
        self.cfg    = self._sub_tree(self.cfg)
        fired = self.cfg.get('assert', [])
        if fired:
            raise ValueError(f"Config assertion failed: {fired[0]}")

    # ---- generators --------------------------------------------------------
    def vsrcs(self, base_dir=None):
        """Verilog source files, env vars expanded, deduped.

        If base_dir is given, each path is rewritten to base_dir/basename so the
        output references staged files (symlinks/copies) rather than originals."""
        seen, out = set(), []
        for s in self._collect(self.cfg.get('design', {}), 'vsrcs'):
            s = os.path.expandvars(s)
            if base_dir:
                s = os.path.join(base_dir, os.path.basename(s))
            if s and s not in seen:
                seen.add(s); out.append(s)
        return out

    def vincdirs(self):
        """Verilog include directories, env vars expanded, deduped."""
        seen, out = set(), []
        for s in self._collect(self.cfg.get('design', {}), 'vincdirs'):
            s = os.path.expandvars(s)
            if s and s not in seen:
                seen.add(s); out.append(s)
        return out

    def defines(self, style):
        """Preprocessor defines for `style`: common 'defines' plus the language
        bucket (verilator/iverilog -> vdefines, gcc -> cdefines). Raw (no -D
        prefix), env vars expanded, empty entries removed."""
        d = self.cfg.get('design', {})
        common = d.get('defines', []) or []
        if style in ('verilator', 'iverilog'):
            extra = d.get('vdefines', []) or []
        elif style == 'gcc':
            extra = d.get('cdefines', []) or []
        else:
            raise ValueError(f"unknown style '{style}' (expected verilator|iverilog|gcc)")
        return [os.path.expandvars(x) for x in common + extra if x]

    def _vcomposite(self, define_prefix, base_dir):
        """Verilog defines + include dirs + sources, in that order. The verilog
        define bucket (common + vdefines) is used; only the define prefix varies
        between the CLI ('-D') and filelist ('+define+') conventions."""
        out  = [f'{define_prefix}{d}' for d in self.defines('verilator')]
        out += [f'+incdir+{p}' for p in self.vincdirs()]
        out += self.vsrcs(base_dir)
        return out

    def vflags(self, base_dir=None):
        """CLI-style verilog invocation: '-D' defines, '+incdir+' dirs, sources.
        For inline use on a verilator/iverilog command line."""
        return self._vcomposite('-D', base_dir)

    def filelist(self, base_dir=None):
        """Standard verilog filelist (command-file): '+define+' macros, '+incdir+'
        dirs and sources -- one per line, for consumption via '-f'. Portable
        across verilator and iverilog."""
        return self._vcomposite('+define+', base_dir)

    # ---- private -----------------------------------------------------------

    def _add_inc(self, path):
        if path not in self.include_paths:
            self.include_paths.append(path)

    def _find_import(self, name):
        for d in self.include_paths:
            for ext in ('.yml', '.yaml'):
                p = d / (name + ext)
                if p.exists():
                    return p
        raise FileNotFoundError(
            f"Import '{name}' not found in: {[str(d) for d in self.include_paths]}")

    def _resolve_imports(self, cfg):
        """Pop 'import', load+merge each recursively. Imports are base; cfg overrides."""
        imports = cfg.pop('import', None) or []
        if isinstance(imports, str):
            imports = [imports]
        base = {}
        for name in imports:
            path = self._find_import(name)
            self._add_inc(path.parent)
            self.files.append(path.resolve())
            with open(path) as f:
                imported = yaml.safe_load(f) or {}
            base = self._deep_merge(base, self._resolve_imports(imported))
        return self._deep_merge(base, cfg)

    def _eval_params(self, raw):
        """Evaluate params in declaration order; derived params may use prior ones."""
        params = {}
        for k, v in raw.items():
            if isinstance(v, str):
                v = evaluate_conditionals(v, params, self.strict)
                if   v.lower() in ('true',  'yes'): v = True
                elif v.lower() in ('false', 'no'):  v = False
                else:
                    try: v = int(v)
                    except ValueError: pass
            params[k] = v
        return params

    def _sub_tree(self, obj):
        """Recursively substitute $[...] in every string in obj."""
        if isinstance(obj, str):  return evaluate_conditionals(obj, self.params, self.strict)
        if isinstance(obj, dict): return {k: self._sub_tree(v) for k, v in obj.items()}
        if isinstance(obj, list): return [r for r in (self._sub_tree(i) for i in obj) if r != '']
        return obj

    @staticmethod
    def _deep_merge(base, override):
        """Layer override on base: dicts merged recursively, lists concatenated
        (base first, deduped), scalars replaced. Lists extend rather than replace
        so a child config adds to the base's defines/sources instead of clobbering them."""
        out = dict(base)
        for k, v in override.items():
            if k in out and isinstance(out[k], dict) and isinstance(v, dict):
                out[k] = Config._deep_merge(out[k], v)
            elif k in out and isinstance(out[k], list) and isinstance(v, list):
                out[k] = Config._merge_lists(out[k], v)
            else:
                out[k] = v
        return out

    @staticmethod
    def _merge_lists(a, b):
        """Concatenate a + b, dropping later duplicates when items are hashable."""
        merged = list(a) + list(b)
        try:
            seen, out = set(), []
            for x in merged:
                if x not in seen:
                    seen.add(x); out.append(x)
            return out
        except TypeError:
            return merged

    @staticmethod
    def _collect(obj, key):
        """Recursively collect all list values stored under key anywhere in obj."""
        found = []
        if isinstance(obj, dict):
            if key in obj and isinstance(obj[key], list):
                found.extend(obj[key])
            for v in obj.values():
                found.extend(Config._collect(v, key))
        return found


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main():
    SUPPORTED_STYLES=['verilator', 'iverilog', 'gcc']
    SUPPORTED_OUTPUTS=['yml', 'vsrcs', 'defines', 'vflags', 'filelist', 'make-vars']

    import argparse
    parser = argparse.ArgumentParser(description='RISC-V Atom YAML config parser')
    parser.add_argument('-c', '--config', help='Top-level YAML config file', 
                        required=True, metavar='FILE')
    parser.add_argument('-I', '--include', help='Add directory to import search path (repeatable)', 
                        action='append', default=[], metavar='DIR')
    parser.add_argument('-s', '--set', help='Override a config key before parsing, e.g. -s params.ext_csr=true',
                        action='append', default=[], metavar='KEY=VAL')
    parser.add_argument('-g', '--get', help='Print a single config value by dotted key, e.g. -g params.isa',
                        default=None, metavar='KEY')
    parser.add_argument('-t', '--type', help=f'Output type: [{" | ".join(SUPPORTED_OUTPUTS)}]',
                        choices=SUPPORTED_OUTPUTS, default=None, metavar='TYPE')
    parser.add_argument('--style', help=f'Output style: [{"| ".join(SUPPORTED_STYLES)}]',
                        default=None, metavar='STYLE', choices=SUPPORTED_STYLES)
    parser.add_argument('--base-dir', help='Rewrite vsrc paths to DIR/basename (use with staged build dirs)',
                        default=None, metavar='DIR')
    parser.add_argument('-o', '--output', help='Write output to file instead of stdout',
                        default=None, metavar='FILE')
    parser.add_argument('--allow-undefined', help='Silently expand undefined parameters to empty string instead of erroring', 
                        action='store_true')
    parser.add_argument('--fno-newlines', help='Drop newlines in output', 
                        action='store_true')
    
    args = parser.parse_args()
    if not args.get and not args.type:
        parser.error("nothing to do: specify -g/--get KEY or -t/--type TYPE")

    cfg = Config()
    for d in args.include:
        cfg._add_inc(Path(d).resolve())

    cfg.strict = not args.allow_undefined
    try:
        cfg.from_file(args.config)
    except (OSError, yaml.YAMLError) as e:
        sys.exit(f"cfgparse: error loading '{args.config}': {e}")

    for kv in args.set:
        if '=' not in kv:
            parser.error(f'--set requires KEY=VALUE, got: {kv!r}')
        k, _, v = kv.partition('=')
        cfg.set(k.strip(), v.strip())

    try:
        cfg.parse()
    except (KeyError, ValueError, FileNotFoundError) as e:
        sys.exit(f"cfgparse: error: {e}")

    if args.get:
        try:
            val = cfg.get(args.get)
        except KeyError:
            sys.exit(f"cfgparse: key not found: '{args.get}'")
        print(val if not isinstance(val, bool) else str(val).lower())
        return

    base_dir = os.path.abspath(args.base_dir) if args.base_dir else None

    # Only 'defines' is language-specific (picks vdefines vs cdefines); the
    # verilog composites (vflags/filelist) always use the verilog bucket.
    # 'make-vars' is self-contained and needs neither --style nor --fno-newlines.
    if args.type == 'defines' and not args.style:
        parser.error("--type defines requires --style (verilator|iverilog|gcc)")

    out = open(args.output, 'w') if args.output else sys.stdout
    sep = ' ' if args.fno_newlines else '\n'
    try:
        match args.type:
            case 'yml':         # Resolved config
                cfg.to_file(out)
            case 'vsrcs':       # Verilog sources only
                out.write(sep.join(cfg.vsrcs(base_dir)) + sep)
            case 'defines':     # -D defines for the given style
                out.write(sep.join(f'-D{d}' for d in cfg.defines(args.style)) + sep)
            case 'vflags':      # CLI-style verilog invocation (-D defines + incdirs + sources)
                out.write(sep.join(cfg.vflags(base_dir)) + sep)
            case 'filelist':    # Standard verilog filelist (+define+ macros + incdirs + sources)
                out.write(sep.join(cfg.filelist(base_dir)) + sep)
            case 'make-vars':  # Make variable assignments for all build-relevant values
                cdefs = ' '.join(f'-D{d}' for d in cfg.defines('gcc'))
                vdefs = ' '.join(f'-D{d}' for d in cfg.defines('verilator'))
                out.write(f'RVATOM_ISA       := {cfg.get("params.isa")}\n')
                out.write(f'RVATOM_ABI       := {cfg.get("params.abi")}\n')
                out.write(f'RVATOM_TOPMODULE := {cfg.get("design.topmodule")}\n')
                out.write(f'RVATOM_CDEFINES  := {cdefs}\n')
                out.write(f'RVATOM_VDEFINES  := {vdefs}\n')
                out.write(f'RVATOM_YMLDEPS   := {" ".join(str(p) for p in cfg.files)}\n')
            case _:
                raise ValueError(f"Invalid output type: {args.type}")
    except OSError as e:
        sys.exit(f"cfgparse: error writing output: {e}")
    finally:
        if args.output:
            out.close()

if __name__ == '__main__':
    try:
        main()
    except (OSError, yaml.YAMLError) as e:
        sys.exit(f"cfgparse: {e}")
