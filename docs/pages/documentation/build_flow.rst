RISC-V Atom Build Flow
#######################

RISC-V Atom project uses a Make base build flow, assisted by the ``cfgparse.py`` script and JSON config files located
under ``rtl/config`` directory. There exists a JSON config file for each / a group of RTL components which specifies
how to compile them, any required flags, macros etc. This implies, each soctarget or a variation of an soctarget has a
JSON which is used to build it.

This approach has many advantages over a static approach of incorporating everything in the Makefiles. The ``cfgparse.py``
is used to parse the json files and extract all the necessary information to build a target like compiler flags, source
files etc. Not only that, cfgparse script allows JSON files to include dependency information on other JSON files, variable
substitution and polymorphism.

Some Information on the Structure of JSON config files
*******************************************************

#. One JSON file can depend on one or more JSON files (specified in ``includes`` attribute), allowing all of them to be
   parsed together.
#. A JSON file can specify parameters in the ``params`` attribute which can be used as variables in conditional statements
   throughout the script.
#. Parameters in the included JSON files can be overridden by the parameters in the JSON file which included it.
#. A conditional statement can be specified as ``[<expression>?<A>:<B>]``, where if the condition is true, ``A`` is
   substituted in the string, else ``B`` is substituted in the string. Users are free to omit A, B, or both depending on
   the requirement.
#. JSON config files support basic polymorphism. They can inherit attributes from the another JSON file using the
   ``extends`` attribute. This is useful when users want to specify a special configuration of an existing configuration
   without duplicating the contents of the original file.
#. Parameters in the *child* config files override parameters in the parent *config* file.
