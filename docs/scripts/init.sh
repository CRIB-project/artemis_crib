#!/bin/sh
# reference: https://sh.rustup.rs

set -u

VERSION="0.1.1"
DATE="2024-01-13"
URL_BASE="https://okawak.github.io/artemis_crib"
ART_ANALYSIS_DIR="${HOME}/art_analysis"

main() {
  need_cmd wget
  need_cmd chmod
  need_cmd mkdir

  if [ ! -d "${ART_ANALYSIS_DIR}" ]; then
    ensure mkdir -p "${ART_ANALYSIS_DIR}"
    say "NEW ${ART_ANALYSIS_DIR}"
  else
    err "Setup is already completed."
  fi

  ensure mkdir -p "${ART_ANALYSIS_DIR}/bin"
  ensure mkdir -p "${ART_ANALYSIS_DIR}/.conf"

  ensure wget -P "${ART_ANALYSIS_DIR}/bin" -nv "${URL_BASE}/bin/art_setting"
  ensure chmod u+x "${ART_ANALYSIS_DIR}/bin/art_setting"

  ensure wget -P "${ART_ANALYSIS_DIR}/bin" -nv "${URL_BASE}/bin/artnew"
  ensure chmod u+x "${ART_ANALYSIS_DIR}/bin/artnew"

  ensure wget -P "${ART_ANALYSIS_DIR}/bin" -nv "${URL_BASE}/bin/art_check"
  ensure chmod u+x "${ART_ANALYSIS_DIR}/bin/art_check"

  ensure wget -P "${ART_ANALYSIS_DIR}/.conf" -nv "${URL_BASE}/bin/artlogin.sh"
  ensure chmod u+x "${ART_ANALYSIS_DIR}/.conf/artlogin.sh"

  usage
}

usage() {
  printf "\n"
  cat <<EOF
art_init: version ${VERSION} (${DATE})
The setup script for CRIB's art_analysis

USAGE:
    add these sentence to the .zshrc or .bashrc
    > export EXP_NAME="expname" # your experiment
    > export EXP_NAME_OLD="expname" # this is option
    > export PATH="\${HOME}/art_analysis/bin:\${PATH}"
    > source \${HOME}/art_analysis/bin/art_setting -q
EOF
}

say() {
  printf "\33[1mart_init\33[0m: %s\n" "$1"
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

# Run a command that should never fail. If the command fails execution
# will immediately terminate with an error showing the failing
# command.
ensure() {
  if ! "$@"; then err "command failed: $*"; fi
}

main "$@" || exit 1
