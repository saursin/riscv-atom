################################################################################
## Dockerfile for RISC-V Atom                                                 ##
################################################################################

FROM ubuntu:20.04
LABEL version=0.1
LABEL description="Docker image for RVAtom environment on ubuntu 18.04"

################################################################################
## Environment Setup

# Setup basic linux env
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    sudo curl git vim nano build-essential ccache python3 python3-pip python-is-python3

# Add new user
# Create a user named "dev" and add to sudo group
RUN useradd -m dev && echo "dev:dev" | chpasswd && adduser dev sudo
WORKDIR /home/dev

# Setup bashrc
RUN curl -L https://gist.githubusercontent.com/saursin/79f60ec68f18fca893848971f4f7e730/raw/b218463c7e001e8f20355d0172c609a11eda3f30/bashrc -o .bashrc

################################################################################
## RISC-V Atom Dev Setup

# Install packages
RUN apt-get install -y --no-install-recommends \
    screen libreadline-dev
RUN pip3 install gdown

# Install RISC-V Toolchain
ARG RVTOOLCHAIN_PREBUILT_FILEID=1kSxCHqrfe7jrk32N3W_e1YbHj97-EsaN
ARG RVTOOLCHAIN_TAR=rv64_mutilib.tar.gz
ARG RVTOOLCHAIN_INSTALL_PATH=/opt/riscv

RUN gdown $RVTOOLCHAIN_PREBUILT_FILEID -O $RVTOOLCHAIN_TAR
RUN mkdir -p $RVTOOLCHAIN_INSTALL_PATH
RUN tar -xvf $RVTOOLCHAIN_TAR -C $RVTOOLCHAIN_INSTALL_PATH && rm -f $RVTOOLCHAIN_TAR

# Install Verilator
ARG VERILATOR_PREBUILT_FILEID=1rXszcRib7oryDU2sMhZpPxMn5Ev3RbxG
ARG VERILATOR_TAR=verilator_5p006.tar.gz
ARG VERILATOR_INSTALL_PATH=/opt/verilator

RUN gdown $VERILATOR_PREBUILT_FILEID -O $VERILATOR_TAR
RUN mkdir -p $VERILATOR_INSTALL_PATH
RUN tar -xvf $VERILATOR_TAR -C $VERILATOR_INSTALL_PATH && rm -f $VERILATOR_TAR

# Add tools to PATH
ENV PATH=$VERILATOR_INSTALL_PATH/bin:$PATH
ENV PATH=$RVTOOLCHAIN_INSTALL_PATH/bin:$PATH

################################################################################
## Setup Repository

# switch to dev user
USER dev
WORKDIR /home/dev

# Setup repo
ARG RVATOM=/home/dev/riscv-atom

# Clone the repo
RUN cd ~ && git clone https://github.com/saursin/riscv-atom.git

# Install repo-specific python packages
RUN pip3 install -r $RVATOM/requirements.txt

# Add sourceme to bashrc
RUN cat <<EOT >> .bashrc
source $RVATOM/sourceme
echo "*** Welcome to the riscv-atom container! ***"
EOT
