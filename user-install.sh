#!/bin/sh

set -e

cd "$(realpath "$(dirname "$0")")"

BUILD_DIR="$PWD/build"
INSTALL_DIR="${1:-$HOME/opt/4ed}"
BIN_DIR="${BINDIR:-$HOME/.local/bin}"

mkdir -p "$INSTALL_DIR"

cp -a "$BUILD_DIR/4ed" \
      "$BUILD_DIR/4ed_app.so" \
      "$BUILD_DIR/custom_4coder.so" \
      "$BUILD_DIR/bindings.4coder" \
      "$BUILD_DIR/config.4coder" \
      "$BUILD_DIR/LICENSE.txt" \
      "$BUILD_DIR/README.txt" \
      "$BUILD_DIR/changes.txt" \
      "$INSTALL_DIR/"

cp -a "$BUILD_DIR/fonts" "$BUILD_DIR/themes" "$INSTALL_DIR/"

mkdir -p "$BIN_DIR"
ln -sf "$INSTALL_DIR/4ed" "$BIN_DIR/4ed"

echo "Installed 4ed to: $INSTALL_DIR"
echo "Linked: $BIN_DIR/4ed -> $INSTALL_DIR/4ed"
