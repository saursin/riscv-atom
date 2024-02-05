RISC-V Atom Development in Docker
##################################

Alternative to previous approach, you can also use the provided `Dockerfile <https://github.com/saursin/riscv-atom/blob/main/Dockerfile>`_
to build a Docker image containing all the necessary tools to checkout the RISC-V Atom project. As a prerequisite, you
must have Docker installed on your system. You can install Docker by following the `official Docker guide <https://docs.docker.com/desktop/>`_.

Once you have installed Docker, you can clone the RISC-V Atom repository and build the Docker image as follows.

.. code-block:: bash

    $ git clone https://github.com/saursin/riscv-atom.git
    $ cd riscv-atom                         # switch to riscv-atom directory
    $ docker build . -t rvatom-dev          # we'll name this image rvatom-dev
 
Once the build is finished you should be able to see the image using the following command:

.. code-block:: bash

    $ docker images
    REPOSITORY   TAG       IMAGE ID       CREATED        SIZE
    rvatom-dev   latest    a9cab48034fc   24 hours ago   475MB

To run an instance of this docker image (also called container), you can run the following command.

.. code-block:: bash

    $ docker run -it -v .:/home/riscv-atom rvatom-dev
    > Setting environment variables...
    *** Welcome to the riscv-atom container! ***
    root@7110d3ddecd7:/home/riscv-atom#

The above command should launch the container and attach to it, and you should have a familiar linux prompt! It will 
also mount the riscv-atom directory on host machine to ``/home/riscv-atom`` directory in the container, and
automatically set-up the environment variables for RISC-V Atom development.

.. tip::
    Checkout this `cheatsheet <https://docs.docker.com/get-started/docker_cheatsheet.pdf>`_ to learn more about Docker CLI syntax.