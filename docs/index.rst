.. RISCV-Atom documentation master file, created by
   sphinx-quickstart on Thu Jul 29 06:43:28 2021.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. image:: diagrams/rvatom-header.png

|

****************************************
RISC-V Atom Documentation & User Manual
****************************************

Welcome to RISC-V Atom Documentation and User Manual! Please follow the :doc:`getting started guide</pages/getting_started/prerequisites>` 
to setup an environment to build and test the RISC-V Atom project. Please feel free file a bug report in github.

.. grid:: 2

   .. grid-item-card::  Github Repository
      :link: https://github.com/saursin/riscv-atom

   .. grid-item-card::  Website
      :link: https://sites.google.com/view/saursin/projects/risc-v-atom?authuser=0


.. toctree::
   :maxdepth: 1
   :caption: Overview

   pages/overview/introduction.rst
   pages/overview/directory_structure.rst


.. toctree::
   :maxdepth: 1
   :caption: Getting Started

   pages/getting_started/prerequisites.rst
   pages/getting_started/building.rst
   pages/getting_started/docker.rst
   pages/getting_started/examples.rst


.. toctree::
   :maxdepth: 1
   :caption: Documentation & User Manual

   pages/documentation/riscv_atom.rst
   pages/documentation/soc_targets.rst
   pages/documentation/memory_map.rst
   pages/documentation/bootloader.rst
   pages/documentation/atomsim/atomsim.rst
   pages/documentation/scar.rst
   pages/documentation/convelf.rst
   pages/documentation/libcatom.rst
   pages/documentation/build_flow.rst
   pages/documentation/performance.rst
   pages/documentation/fpga_results.rst


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
