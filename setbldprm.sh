#!/bin/bash

#===== USER SETTING ======================================
# need this format!
mwdcs=(
   "mwdca"
   "mwdcb"
)
#=========================================================

arthome=$(
   cd "$(dirname "$0")" || exit 1
   pwd
)
time=$(date)

usage() {
   printf "Set the symbolic link for BLD parameter\n"
   printf "Before use this shellscript, please check the MWDC conf in this file!\n\n"
   printf "\033[1m\033[4mUsage:\033[0m $ ./setbldprm.sh [ARGUMENT or OPTION]\n\n"
   printf "\033[1m\033[4mArguments:\033[0m\n"
   printf "  -h        Print help\n"
   printf "  -c        Clean the parameter files (delete old experiment files)\n"
   printf "  -l        Print available parameters\n"
   printf "  run0000   The name of parameter files. Set this parameter\n"
}

prm_clean() {
   while true; do
      read -rp "WARNING: all the BLD parameter files will be cleaned, is it okay? (y/n): " _answer
      case ${_answer} in
      y)
         say "clean the parameter files..."

         for mwdc in "${mwdcs[@]}"; do
            mwdc_prm_dir="$arthome/prm/mwdc/$mwdc/dt2dl"
            link_file=$(readlink -f "$mwdc_prm_dir/current")
            for file in "$mwdc_prm_dir"/*; do
               if [ "$file" = "$link_file" ] || [ "$file" = "$mwdc_prm_dir/current" ]; then
                  continue
               fi
               say "rm $file"
               rm -rf "$file"
            done
         done

         break
         ;;
      n)
         say "cancelled"
         break
         ;;
      esac
   done
}

show_list() {
   say "avaliable parameter list:"
   for mwdc in "${mwdcs[@]}"; do
      say "$mwdc:"
      mwdc_prm_dir="$arthome/prm/mwdc/$mwdc/dt2dl"
      cd "$mwdc_prm_dir" || exit 1
      for file in *; do
         if [ "$file" = "current" ]; then
            continue
         fi
         say " - $file"
      done
      printf "\n"
   done
   cd "$arthome" || exit 1
}

main() {
   if [ "$1"_ = _ ]; then
      err "need correct argument"
   fi

   if [ $# -ne 1 ]; then
      err "need correct argument"
   fi

   for mwdc in "${mwdcs[@]}"; do
      mwdc_prm_dir="$arthome/prm/mwdc/$mwdc/dt2dl"
      cd "$mwdc_prm_dir" || exit 1
      if [ -d "$1" ]; then
         say "$mwdc pamameter: $mwdc_prm_dir/$1 is current"
         rm -f current
         ln -sf "$1" current
      else
         err "$mwdc_prm_dir/$1 not found."
      fi
   done

   cd "$arthome" || exit 1
   echo "${time} using $1 prm" >>"$arthome/.log_mwdc"
}

say() {
   printf "\33[1msetmuxprm.sh\33[0m: %s\n" "$1"
}

err() {
   say "$1" >&2
   usage
   exit 1
}

# script start
while getopts "hcl" OPT; do
   case $OPT in
   h)
      usage
      exit 0
      ;;
   c)
      prm_clean
      exit 0
      ;;
   l)
      show_list
      exit 0
      ;;
   \?)
      err "invalid option!"
      ;;
   esac
done

main "$@"
