#!/bin/bash

# This script assume to run in local environment,
# not via "curl" command
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

main() {
  # check necessary tools are installed
  need_cmd mdbook
  need_cmd mdbook-mermaid

  # test markdown files using mdbook test
  echo "Testing mdbook..."
  cd "$SCRIPT_DIR/guide"
  mdbook-mermaid install .
  mdbook test
  cd -
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
