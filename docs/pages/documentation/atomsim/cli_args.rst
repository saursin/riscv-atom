AtomSim CLI Argument Reference
###############################
The following are the arguments that may be passed to the AtomSim executable.

.. code-block:: text
   
   Usage:
   $ atomsim [OPTION...] input_file

   Config options:
      --maxitr arg      Specify maximum simulation iterations (default: 10000000)
      --vuart arg       use provided virtual uart port (default: Null)
      --vuart-baud arg  Specify virtual uart port baudrate (default: 9600)

   Debug options:
   -v, --verbose        Turn on verbose output
   -d, --debug          Start in debug mode
   -t, --trace          Enable VCD tracing 
       --trace-dir arg  Specify trace directory (default: build/trace)
       --dump-dir arg   Specify dump directory (default: build/trace)
       --ebreak-dump    Enable processor state dump at hault
       --signature arg  Enable signature sump at hault (Used for riscv 
                        compliance tests) (default: "")

   General options:
   -h, --help           Show this message
       --version        Show version information
       --simtarget      Show current AtomSim Target
   -i, --input arg      Specify an input file
