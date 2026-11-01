SCAR: Search Compile Assert Run
################################

SCAR unit-tests the processor one instruction at a time. It is a pytest suite in ``RVATOM/test/scar``:
``test_instructions.py`` runs every test listed in ``tests.yml``.

SCAR Workflow
**************
For each test, SCAR:

#. **Compiles** the test's assembly sources (``tests/*.S``), linked with the target's linker script from the libcatom
   build (``<build>/sw/lib/link_<soctarget>.ld``).
#. **Runs** the ELF on atomsim with ``--exit-dump``, which dumps the registers when the program reaches the exit hint
   ``slti x0, x0, 1``. Outputs go to the ``work`` directory.
#. **Asserts** the expressions at the end of the assembly file against the register dump.

Run it from the ``RVATOM`` directory with the ``scar`` target, which builds atomsim first:

.. code-block:: bash

  $ make soctarget=hydrogensoc scar

Or run pytest directly, pointing ``ATOMSIM`` at the target's simulator (without it, the ``atomsim`` on ``PATH`` is used,
and the tests are skipped if there is none):

.. code-block:: bash

  $ cd test/scar
  $ export ATOMSIM=$RVATOM/build/hydrogensoc/sim/bin/atomsim
  $ pytest -v                 # all tests
  $ pytest -v -k add          # filter by name

``tests.yml`` groups the tests by the ISA extension they need (``i``, ``m``, ...). A group whose extension the target
lacks is skipped; SCAR reads the target ISA from the build (``RVATOM_ISA`` in ``vars.<cfg>.mk``). On atombones
the M-extension tests are skipped.

How to Create Your Own Tests?
******************************
#. The assembly file needs a ``_start`` label before the code and the exit hint ``slti x0, x0, 1`` after it; atomsim
   stops there.
#. After the exit hint, add an assertion block: Python expressions over register names, either physical (``x0``,
   ``x1``, ...) or ABI (``zero``, ``ra``, ``sp``, ...), one per line:

   .. code-block:: asm

          li t0, 0xFF
          li x3, 0x01234567

          slti x0, x0, 1    # exit hint: atomsim stops here
      #>>>ASSERT
      # t0 == 0x000000FF
      # x3 == 0x01234567
      #<<<END_ASSERT

#. Save it as ``tests/<name>.S`` and add the name to ``tests.yml``, under the extension it needs:

   .. code-block:: yaml

      tests:
        i:
          - addi
