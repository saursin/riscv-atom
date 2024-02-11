RISC-V Atom Development in Docker
##################################

Alternative to previous approach, you can also use the provided `Dockerfile <https://github.com/saursin/riscv-atom/blob/main/Dockerfile>`_
to build a Docker image containing all the necessary tools to checkout the RISC-V Atom project. This approach is OS agnostic.
As a prerequisite, you must have Docker installed on your system. You can install Docker by following the
`official Docker guide <https://docs.docker.com/desktop/>`_.

Once you have installed Docker, you can follow these steps to setup a RISC-V Atom dev environment.

.. tip::
  We recommend using bash is you have a Linux OS and powershell if you're on Windows.


1. Obtain the Dockerfile from the RISC-V Atom repository either manually or by using the following command.

.. code-block:: bash
  
  $ wget https://raw.githubusercontent.com/saursin/riscv-atom/main/Dockerfile
  $ ls                                              # check if the file was downloaded

2. Once you have the Dockerfile, we need to build a docker image as follows.

.. code-block:: bash

  $ docker build . -t rvatom-dev-img                # We'll call this image rvatom-dev-img
  $ docker images                                   # List docker images
  REPOSITORY       TAG       IMAGE ID       CREATED          SIZE
  rvatom-dev-img   latest    4e888efaf4c1   3 minutes ago    2.46GB

4. Now we can create a new instance of this image (aka container) called ``rvatom`` from ``rvatom-dev-img`` image as follows.

.. code-block:: bash

  $ docker create -it --name rvatom rvatom-dev-img  # We'll call this container rvatom
  $ docker ps -a                                    # List all containers (running or not running)
  CONTAINER ID   IMAGE            COMMAND       CREATED         STATUS    PORTS     NAMES
  5a1a95325d61   rvatom-dev-img   "/bin/bash"   5 seconds ago   Created             rvatom

5. At this point, we're all set to start our new docker container as follows!

.. code-block:: bash
  
  $ docker start -i rvatom
  ***** RISC-V Atom Environment Setup *****
  > Setting environment variables...
  Found Verilator at: /opt/verilator
  *** Welcome to the riscv-atom container! ***
  ~@dev $

You should be able to see a bash prompt, and the riscv-atom repository already cloned for you in the home folder!

.. tip::
  You can detach from the container using :kbd:`ctrl` + :kbd:`d` or by using the ``exit`` command. This also stops the
  container. To restart, simpy use the above ``docker start`` command again.

.. tip::
  Checkout this `cheatsheet <https://docs.docker.com/get-started/docker_cheatsheet.pdf>`_ to learn more about Docker CLI syntax.

.. tip::
  If you want to use VS Code for development, install the **Remote Explorer** and **Dev Containers** extensions. You
  should now be able to see the "Dev Containers" options in the *Remote Containers* tab and the container we just created
  in the list.