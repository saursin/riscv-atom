RISC-V Atom Build Flow
#######################

RISC-V Atom uses a Make based build flow, driven by YAML config files under ``rtl/config`` and the ``cfgparse.py``
script. Each soctarget has a config (``<soctarget>.yml``), and a board can override it (``<soctarget>-<board>.yml``).
The config is the only source of the target's ISA, RTL sources and memory map; the Makefiles take everything from it
(see :doc:`../getting_started/building` for how the build uses it).

Structure of the YAML config files
**********************************

#. A config can ``import`` one or more other configs. They are deep-merged: the importing config overrides the
   values it sets, and lists (defines, sources) are appended.
#. ``params`` holds parameters. Derived parameters may use earlier ones, and size strings such as ``8KB`` or
   ``80MB`` become byte counts.
#. ``params.peripherals`` holds the memory map and peripherals. Every ``<x>.<key>`` becomes ``SOC_<X>_<KEY>`` in the
   generated config headers; ``enable: true`` becomes ``SOC_EN_<X>``. A disabled peripheral keeps its keys (its
   address slot is part of the map) but gets no ``SOC_EN_<X>``, so no hardware.
#. ``design`` holds the top module, extra defines (``defines`` for all languages, ``vdefines`` for Verilog,
   ``cdefines`` for C) and the RTL sources (``vsrcs``, ``vincdirs``).
#. String values may use templates: ``$[param]`` (dotted paths for nested values), ``$[env.VAR]``,
   ``$[shell CMD]`` and ``$[expr?A:B]``, where ``expr`` is a Python expression over the params and ``A`` or ``B``
   may be empty.
#. ``assert`` entries fail the build when their condition fires.
