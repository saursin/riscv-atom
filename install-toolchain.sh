#!/bin/bash
# set -x # display execution trace
########################################################
# Toolchain name
TOOLCHAIN_NAME=rv64gc-multilib
TOOLCHAIN_CONFIG=--enable-multilib
BUILD_JOBS=4

# Toolchain install root directory (Note: Toolchain will be installed inside a folder named TOOLCHAIN_NAME in the TOOLCHAIN_ROOTPATH)
TOOLCHAIN_ROOTPATH=/home/frozenalpha/opt/riscv

#########################################################
# You sould not change anything below this

TOOLCHAIN_INSTALL_PATH=${TOOLCHAIN_ROOTPATH}/${TOOLCHAIN_NAME}
BUILD_DIR=build/${TOOLCHAIN_NAME}

CWDIR=$(pwd)

RED="\e[31m"
GREEN="\e[32m"
ORANGE="\e[33m"
NOCOLOR="\e[0m"


# Checking for any existing toolchain installation
echo -e "${GREEN}Checking for any existing toolchains...${NOCOLOR}"
if [ -d ${TOOLCHAIN_INSTALL_PATH} ]
then
    echo "Found an existing installation of ${TOOLCHAIN_NAME}"

    read -p "This operation will remove it completely, Are you sure? [y/n] " -n 1 -r
    echo    # (optional) move to a new line
    if [[ $REPLY =~ ^[Yy]$ ]]
    then
        # Remove installation
        echo -e "${GREEN}Removing existing toolchain... ${NOCOLOR}"
        rm -rf ${TOOLCHAIN_INSTALL_PATH}/*
    else
        echo -e "${RED}Abort ${NOCOLOR}"
        [[ "$0" = "$BASH_SOURCE" ]] && exit 1 || return 1
    fi
else
    echo "Not found"

    # Create installation directory
    mkdir -p ${TOOLCHAIN_INSTALL_PATH}
fi

echo -e "${GREEN}Building toolchain... ${NOCOLOR}"
# Create build directory
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

# Build toolchain
cd ${BUILD_DIR}

echo "../../configure --prefix=$(pwd) ${TOOLCHAIN_CONFIG}"
../../configure --prefix=$(pwd) ${TOOLCHAIN_CONFIG}

echo "make -j${BUILD_JOBS}"
make -j${BUILD_JOBS}

# Copy files to the installation directory
echo -e "${GREEN}Copying... ${NOCOLOR}"
rsync -r --progress * ${TOOLCHAIN_INSTALL_PATH}/

cd ${CWDIR}

echo "*** Toolchain Installation Successful! ***"
echo "Toolchain        : ${TOOLCHAIN_NAME}"
echo "Install location : ${TOOLCHAIN_INSTALL_PATH}"
echo ""
echo "You should now append the following lines to your \".bashrc\" file"
echo "  export PATH=\"${TOOLCHAIN_INSTALL_PATH}/bin:\$PATH\""