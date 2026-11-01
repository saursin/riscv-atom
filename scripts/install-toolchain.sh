#!/bin/bash
# Installs the riscvtoolkit release (RISC-V GCC toolchain, Verilator) into <prefix>/riscvtoolkit.
# usage: scripts/install-toolchain.sh [prefix]      (default prefix: tools/ in this repo)
set -euo pipefail

VERSION=2026.11
NAME=riscvtoolkit-$VERSION-linux-x86_64.tar.xz
URL=https://github.com/saursin/riscvtoolkit/releases/download/$VERSION

PREFIX=${1:-$(dirname "$(dirname "$(readlink -f "$0")")")/tools}
DEST=$PREFIX/riscvtoolkit

if [ -e "$DEST" ]; then
    echo "$DEST already exists; remove it first to reinstall"
    exit 1
fi

mkdir -p "$PREFIX"
DL=$(mktemp -d "$PREFIX/.download.XXXXXX")
trap 'rm -rf "$DL"' EXIT

echo "Downloading riscvtoolkit $VERSION"
curl -fL --progress-bar -o "$DL/$NAME" "$URL/$NAME"
curl -fsSL -o "$DL/$NAME.sha256" "$URL/$NAME.sha256"
(cd "$DL" && sha256sum -c "$NAME.sha256")
tar -xJf "$DL/$NAME" -C "$PREFIX" --no-same-owner
echo "Installed riscvtoolkit $VERSION in $DEST"
