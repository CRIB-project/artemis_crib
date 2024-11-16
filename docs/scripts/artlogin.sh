#!/bin/bash

if [ -z "$1" ]; then
  username=${ART_EXP_NAME}
else
  username=$1
fi

userdir=${ART_ANALYSIS_DIR}/${ART_EXP_NAME}/${username}

if [ ! -d "${ART_ANALYSIS_DIR}/${ART_EXP_NAME}" ]; then
  printf "please make new %s work directory by the \"artnew\" command\n" "${ART_EXP_NAME}"
  return 1
fi

if [ -d "${userdir}" ]; then
  cd "${userdir}" || return 1
  ART_USER_FULLNAME="$(git config user.name)"
  export ART_USER_FULLNAME
  ART_USER_EMAIL="$(git config user.email)"
  export ART_USER_EMAIL
  export ARTEMIS_WORKDIR="${userdir}"
  export ARTEMIS_USER="${username}"
  alias acd='cd ${ARTEMIS_WORKDIR}'

  #if [ -z "$1" ]; then
  #    printf "[info] default user, please don't change anything in this directory\n"
  #fi

  if [ -e "${ARTEMIS_WORKDIR}/thisartemis-crib.sh" ]; then
    # shellcheck disable=SC1091
    source "${ARTEMIS_WORKDIR}/thisartemis-crib.sh"
  fi

  return 0
elif [ -e "${userdir}" ]; then
  printf "\33[1martlogin\33[0m: strange file %s exist.\n" "${userdir}"
  return 1
fi

printf "\33[1martlogin\33[0m: user '%s' not found.\n" "${username}"
while true; do
  printf "create new user? (y/n): "
  read -r _answer
  case ${_answer} in
  y)
    break
    ;;
  n)
    echo "cancelled."
    return 0
    ;;
  esac
done

export ARTEMIS_WORKDIR=${userdir}
export ARTEMIS_USER=${username}
git clone "${ART_USER_REPOS}" "${userdir}"

cd "${userdir}" || return 1
alias acd='cd ${ARTEMIS_WORKDIR}'

printf "\33[1martlogin\33[0m: making local git config\n"
while true; do
  printf "Input fullname: "
  read -r _fullname
  printf "Is it Okay? (y/n): "
  read -r _answer
  case ${_answer} in
  y)
    break
    ;;
  esac
done

git config user.name "${_fullname}"

while true; do
  printf "Input email address: "
  read -r _email
  printf "Is it Okay? (y/n): "
  read -r _answer
  case ${_answer} in
  y)
    break
    ;;
  esac
done

git config user.email "${_email}"

if [ -d "${ART_DATA_DIR}" ]; then
  ln -fs "${ART_DATA_DIR}" "${ARTEMIS_WORKDIR}/ridf"
else
  printf "\33[1martlogin\33[0m: %s is not existed.\n" "${ART_DATA_DIR}"
fi

if [ ! -d "${OUTPUT_PATH}/${username}/output" ]; then
  mkdir -p "${OUTPUT_PATH}/${username}/output"
fi

if [ ! -d "${OUTPUT_PATH}/${username}/rootfile" ]; then
  mkdir -p "${OUTPUT_PATH}/${username}/rootfile"
fi

ln -fs "${OUTPUT_PATH}/${username}/output" "${ARTEMIS_WORKDIR}/output"
ln -fs "${OUTPUT_PATH}/${username}/rootfile" "${ARTEMIS_WORKDIR}/rootfile"

printf "before starting artemis,\n"
printf "> mkdir build && cd build\n"
printf "> cmake ..\n"
printf "> make -j4\n"
printf "> make install\n"
printf "> acd\n"
