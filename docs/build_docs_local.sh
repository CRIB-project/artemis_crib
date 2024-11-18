#!/bin/bash

# This script assume to run in local environment,
# not via "curl" command
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

build_mdbook() {
  echo "Building mdBook..."
  cd "$SCRIPT_DIR"/guide
  rm -rf book
  mdbook build
  cd "$SCRIPT_DIR"
}

build_doxygen() {
  echo "Generating Doxygen docs..."
  cd "$SCRIPT_DIR"/reference
  rm -rf html
  doxygen Doxyfile
  cd "$SCRIPT_DIR"
}

main() {
  # check necessary tools are installed
  need_cmd mdbook
  need_cmd doxygen
  need_cmd uv

  # build mdBook
  build_mdbook

  # build doxygen
  build_doxygen

  # combine the pages to public directory
  echo "Combining outputs into 'public/'..."
  cd "$SCRIPT_DIR"/..
  rm -rf public
  mkdir -p public
  cp -r docs/guide/book/* public/
  cp -r docs/reference/html public/reference/

  # start local server by using python
  echo "Serving at http://localhost:8080"
  cd public
  uv run python -m http.server 8080
}

say() {
  printf "\33[1mbuild docs\33[0m: %s\n" "$1"
}

err() {
  say "$1" >&2
  exit 1
}

need_cmd() {
  if ! check_cmd "$1"; then
    err "need '$1' (command not found)"
  fi
}

check_cmd() {
  command -v "$1" >/dev/null 2>&1
}

main "$@" || exit 1
