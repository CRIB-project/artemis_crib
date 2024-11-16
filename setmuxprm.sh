#!/bin/bash

#===== USER SETTING ======================================
# need this format! [mux_id, tel_name, dEX or dEY]
# The delimiter must be a space
mux_usage=(
   "MUX1-1 tel6 dEX"
   "MUX1-2 tel6 dEY"
   "MUX2-1 tel5 dEX"
   "MUX2-2 tel5 dEY"
   "MUX3-1 tel4 dEX"
   "MUX3-2 tel4 dEY"
)
#=========================================================

arthome=$(
   cd "$(dirname "$0")" || exit 1
   pwd
)
time=$(date)

usage() {
   printf "Set the symbolic link for MUX position parameter\n"
   printf "Before use this shellscript, please check the MUX conf in this file!\n\n"
   printf "\033[1m\033[4mUsage:\033[0m $ ./setmuxprm.sh [ARGUMENT or OPTION]\n\n"
   printf "\033[1m\033[4mArguments:\033[0m\n"
   printf "  -h        Print help\n"
   printf "  -c        Clean the parameter files (delete old experiment files)\n"
   printf "  -l        Print available parameters\n"
   printf "  run0000   The name of parameter files. Set this parameter\n"
}

prm_clean() {
   while true; do
      read -rp "WARNING: all the MUX parameter files will be cleaned, is it okay? (y/n): " _answer
      case ${_answer} in
      y)
         say "clean the parameter files..."

         for mux_info in "${mux_usage[@]}"; do
            read -ra mux <<<"${mux_info}"
            mux_prm_dir="$arthome/prm/${mux[1]}/pos_${mux[2]}"
            link_file=$(readlink -f "$mux_prm_dir/current")
            for file in "$mux_prm_dir"/*; do
               if [ "$file" = "$link_file" ] || [ "$file" = "$mux_prm_dir/current" ]; then
                  continue
               fi
               say "rm $file"
               rm -f "$file"
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
   for mux_info in "${mux_usage[@]}"; do
      read -ra mux <<<"${mux_info}"
      say "$mux_info:"
      mux_prm_dir="$arthome/prm/${mux[1]}/pos_${mux[2]}"
      cd "$mux_prm_dir" || exit 1
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

   for mux_info in "${mux_usage[@]}"; do
      read -ra mux <<<"${mux_info}"

      mux_prm_dir="$arthome/prm/${mux[1]}/pos_${mux[2]}"
      cd "$mux_prm_dir" || exit 1
      if [ -f "$1.dat" ]; then
         say "${mux[0]} ${mux[1]} ${mux[2]}: $mux_prm_dir/$1.dat is current"
         rm -f current
         ln -sf "$1.dat" current
      else
         err "$mux_prm_dir/$1 not found."
      fi
   done

   cd "$arthome" || exit 1
   echo "${time} using $1 prm" >>"$arthome/.log_mux"
}

say() {
   printf "\33[1msetmuxprm.sh\33[0m: %s\n" "$1"
}

err() {
   say "$1" >&2
   usage
   exit 1
}

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
