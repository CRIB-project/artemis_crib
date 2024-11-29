#!/bin/sh

# Exit immediately if a command exits with a non-zero status,
# and treat unset variables as an error.
set -eu

# Enable 'pipefail' option if the shell supports it.
if (set -o | grep -q pipefail); then
  set -o pipefail
fi

# Script version information
VERSION="0.2.0"
DATE="2024-11-30"

# Base URL for downloading scripts
URL_BASE="https://crib-project.github.io/artemis_crib"

# Directory where the analysis tools will be installed
ART_ANALYSIS_DIR="${HOME}/art_analysis"

main() {
  # Check for required commands: wget or curl, chmod, mkdir
  if ! need_cmd "wget" && ! need_cmd "curl"; then
    err "Neither 'wget' nor 'curl' is available."
  fi
  need_cmd "chmod"
  need_cmd "mkdir"

  # Check if the analysis directory already exists
  if [ -d "${ART_ANALYSIS_DIR}" ]; then
    err "Setup is already completed."
  else
    # Create the art_analysis directory
    ensure mkdir -p "${ART_ANALYSIS_DIR}"
    say "Created directory: ${ART_ANALYSIS_DIR}"
  fi

  # Create subdirectories for binaries and configuration files
  ensure mkdir -p "${ART_ANALYSIS_DIR}/bin"
  ensure mkdir -p "${ART_ANALYSIS_DIR}/.conf"

  # Download scripts and set executable permissions
  download_and_chmod "scripts/art_setting" "bin/art_setting"
  download_and_chmod "scripts/artnew" "bin/artnew"
  download_and_chmod "scripts/artup" "bin/artup"
  download_and_chmod "scripts/artlogin.sh" ".conf/artlogin.sh"

  # Display usage information
  usage
}

usage() {
  cat <<EOF

art_init: version ${VERSION} (${DATE})
The setup script for CRIB's art_analysis

USAGE:
    Add the following lines to your .zshrc or .bashrc:

    export EXP_NAME="expname"     # your experiment name
    export EXP_NAME_OLD="expname" # this is optional
    export PATH="\${HOME}/art_analysis/bin:\${PATH}"
    source \${HOME}/art_analysis/bin/art_setting -q

EOF
}

# Function to print messages in bold
say() {
  printf "\033[1mart_init\033[0m: %s\n" "$1"
}

# Function to print error messages and exit
err() {
  say "$1" >&2
  exit 1
}

# Function to check if a command exists
need_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    say "Command not found: $1"
    return 1
  fi
}

# Function to ensure a command runs successfully
ensure() {
  if ! "$@"; then
    err "command failed: $*"
  fi
}

# Function to download a file and set executable permissions
download_and_chmod() {
  dc_relative_url="$1"  # URL path relative to URL_BASE
  dc_relative_path="$2" # Destination path relative to ART_ANALYSIS_DIR
  dc_url="${URL_BASE}/${dc_relative_url}"
  dc_dest="${ART_ANALYSIS_DIR}/${dc_relative_path}"

  # Check if the file already exists
  if [ -f "${dc_dest}" ]; then
    say "File already exists: ${dc_dest}"
  else
    # Use wget or curl to download the file
    if command -v wget >/dev/null 2>&1; then
      ensure wget -q -O "${dc_dest}" "${dc_url}"
    elif command -v curl >/dev/null 2>&1; then
      ensure curl -fsSL -o "${dc_dest}" "${dc_url}"
    else
      err "Neither wget nor curl is available."
    fi
    # Set executable permissions on the downloaded file
    ensure chmod u+x "${dc_dest}"
    say "Downloaded and set executable: ${dc_dest}"
  fi
}

# Execute the main function
main "$@"
