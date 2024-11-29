#!/bin/sh

# This script sets up the Artemis environment for a specific user.
# It is intended to be sourced within the 'artlogin' or 'artlogin2' functions defined in 'art_setting'.

# Get the username from the environment variable
if [ -n "${ARTLOGIN_USERNAME:-}" ]; then
  username="${ARTLOGIN_USERNAME}"
else
  username="${ART_EXP_NAME}"
fi

# Define the user's directory
userdir="${ART_ANALYSIS_DIR}/${ART_EXP_NAME}/${username}"

# Check if the experiment directory exists
if [ ! -d "${ART_ANALYSIS_DIR}/${ART_EXP_NAME}" ]; then
  printf "Please create the '%s' work directory using the 'artnew' command.\n" "${ART_EXP_NAME}"
  return 1
fi

# If the user's directory exists
if [ -d "${userdir}" ]; then
  # Navigate to the user's directory
  cd "${userdir}" || return 1

  # Retrieve and export user's Git configuration
  ART_USER_FULLNAME="$(git config user.name)"
  export ART_USER_FULLNAME
  ART_USER_EMAIL="$(git config user.email)"
  export ART_USER_EMAIL

  # Set environment variables for Artemis
  export ARTEMIS_WORKDIR="${userdir}"
  export ARTEMIS_USER="${username}"
  alias acd='cd "${ARTEMIS_WORKDIR}"'

  # Source additional Artemis configurations if available
  if [ -e "${ARTEMIS_WORKDIR}/thisartemis-crib.sh" ]; then
    . "${ARTEMIS_WORKDIR}/thisartemis-crib.sh"
  fi

  return 0

# If the user's directory exists but is not a directory
elif [ -e "${userdir}" ]; then
  printf "\033[1martlogin\033[0m: Strange file '%s' exists.\n" "${userdir}"
  return 1
fi

# If the user's directory does not exist
printf "\033[1martlogin\033[0m: User '%s' not found.\n" "${username}"

# Prompt to create a new user
while true; do
  printf "Create new user? (y/n): "
  read -r answer
  case "${answer}" in
  y | Y)
    break
    ;;
  n | N)
    echo "Cancelled."
    return 0
    ;;
  *)
    echo "Please answer 'y' or 'n'."
    ;;
  esac
done

# Set environment variables for the new user
export ARTEMIS_WORKDIR="${userdir}"
export ARTEMIS_USER="${username}"

# Clone the user's repository
git clone --branch "${ART_USER_REPOS_BRANCH}" "${ART_USER_REPOS}" "${userdir}"

# Navigate to the user's directory
cd "${userdir}" || return 1
alias acd='cd "${ARTEMIS_WORKDIR}"'

# Configure local Git settings
printf "\033[1martlogin\033[0m: Configuring local git settings.\n"

# Prompt for the user's full name
while true; do
  printf "Input full name: "
  read -r fullname
  printf "Is this correct? (y/n): "
  read -r confirm
  case "${confirm}" in
  y | Y)
    break
    ;;
  *) ;;
  esac
done
git config user.name "${fullname}"

# Prompt for the user's email address
while true; do
  printf "Input email address: "
  read -r email
  printf "Is this correct? (y/n): "
  read -r confirm
  case "${confirm}" in
  y | Y)
    break
    ;;
  *) ;;
  esac
done
git config user.email "${email}"

# Link the data directory if it exists
if [ -d "${ART_DATA_DIR}" ]; then
  ln -fs "${ART_DATA_DIR}" "${ARTEMIS_WORKDIR}/ridf"
else
  printf "\033[1martlogin\033[0m: Data directory '%s' does not exist.\n" "${ART_DATA_DIR}"
fi

# Create output directories if they do not exist
if [ ! -d "${OUTPUT_PATH}/${username}/output" ]; then
  mkdir -p "${OUTPUT_PATH}/${username}/output"
fi
if [ ! -d "${OUTPUT_PATH}/${username}/rootfile" ]; then
  mkdir -p "${OUTPUT_PATH}/${username}/rootfile"
fi

# Create symbolic links to the output directories
ln -fs "${OUTPUT_PATH}/${username}/output" "${ARTEMIS_WORKDIR}/output"
ln -fs "${OUTPUT_PATH}/${username}/rootfile" "${ARTEMIS_WORKDIR}/rootfile"

# Display instructions for building Artemis
printf "Before starting Artemis, execute the following commands:\n"
printf "> mkdir build && cd build\n"
printf "> cmake ..\n"
printf "> make -j4\n"
printf "> make install\n"
printf "> acd\n"
