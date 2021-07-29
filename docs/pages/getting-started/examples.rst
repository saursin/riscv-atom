**********
Examples
**********
Atom comes with a wide range of examples programs out-of-the-box to test. These examples programs reside 
in `RVATOM/sw/examples` directory.

Lets jump to the classical "hello World!" example first!

Hello World
------------
First we need to compile the hello world example with our RISCV gcc compiler. For this purpose, use the 
provided compile_atombones shell script as following.

::

  $ compile_atombones.sh sw/examples/hello/hello.s -o build/hello.elf

The above command should generate a `hello.elf` file in the `build directory`. Now fire up atomsim and 
provide the generated elf file as argument.

::

  $ atomsim build/hello.elf

This should output:
::

  Hello World!
  -- AtomBones

We can test other examples also in the similar fashion.

Banner
-------
::

  $ compile_atombones.sh sw/examples/banner/banner.c -o build/banner.elf
  $ atomsim build/banner.elf

output:
::
                    .';,.           ....;;;.  
                   .ll,:o,                ':c,. 
                   .dd;co'                  .cl,  
                  .:o:;,.                     'o:  
                  co.                          .oc  
                 ,o'          .coddoc.          'd,  
                 lc         .lXMMMMMMXl.         ll  
                .o:         ;KMMMMMMMMK,         :o. 
                .o:         'OMMMMMMMMO.         :o. 
                 co.         .o0XNNX0o.         .oc  
                 .o:           ..''..           :o.  
                  'o:                          :o'  
                   .lc.                      .ll.  
                     ,lc'                  'cl,   
                       'cc:,..        ..,:c:'   
                          .;::::;;;;::::;.    
                                ....        
       ____  _________ _______    __         __                 
      / __ \/  _/ ___// ____/ |  / /  ____ _/ /_____  ____ ___  
     / /_/ // / \__ \/ /    | | / /  / __ `/ __/ __ \/ __ `__ \ 
    / _, _// / ___/ / /___  | |/ /  / /_/ / /_/ /_/ / / / / / /      
   /_/ |_/___//____/\____/  |___/   \__,_/\__/\____/_/ /_/ /_/  
  /=========By: Saurabh Singh (saurabh.s99100@gmail.com)====/
  
  UART tx at : 0x08000001
  UART rx at : 0x08000000


Runall Script
-------------
Instead of testing all examples one-by-one, we can use the provided `atomsim-runall.sh` script to 
autometically compile and simulate all examples.

::

  $ atomsim-runall.sh
