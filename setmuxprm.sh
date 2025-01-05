#!/bin/bash

# Global variables
tel_names=("tel1" "tel2" "tel3" "tel4" "tel5" "tel6")
side_names=("dEX" "dEY")

# Utility functions
say() {
  printf "\33[1msetmuxprm.sh\33[0m: %s\n" "$1"
}

err() {
  say "$1" >&2
  exit 1
}

need_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    err "Command not found: $1"
  fi
}

get_relative_path() {
  realpath --relative-to="$ARTEMIS_WORKDIR" "$1"
}

check_dir() {
  local dir="$1"
  if [ ! -d "$dir" ]; then
    say "Directory not found: $(get_relative_path "$dir"). Skipping..."
    return 1
  fi
}

# Clean parameters
prm_clean() {
  local deleted_files=()

  if gum confirm "WARNING: All the MUX parameter files will be cleaned. Continue?"; then
    for tel_name in "${tel_names[@]}"; do
      for side_name in "${side_names[@]}"; do
        local mux_prm_dir="$ARTEMIS_WORKDIR/prm/$tel_name/pos_$side_name"
        check_dir "$mux_prm_dir" || continue

        local link_file
        link_file=$(readlink -f "$mux_prm_dir/current")
        for file in "$mux_prm_dir"/*; do
          if [[ "$file" == "$mux_prm_dir/test.dat" || "$file" == "$link_file" || "$file" == "$mux_prm_dir/current" ]]; then
            continue
          fi
          gum spin --title "Removing $file" -- rm -f "$file"
          deleted_files+=("$file")
        done
      done
    done

    if [ ${#deleted_files[@]} -gt 0 ]; then
      say "Deleted files:"
      printf "  %s\n" "${deleted_files[@]}"
    else
      say "No files were deleted."
    fi
  else
    say "Clean operation cancelled."
  fi
}

show_list() {
  say "Available parameter list:"
  for tel_name in "${tel_names[@]}"; do
    for side_name in "${side_names[@]}"; do
      local mux_prm_dir="$ARTEMIS_WORKDIR/prm/$tel_name/pos_$side_name"
      check_dir "$mux_prm_dir" || continue

      local link_file
      link_file=$(readlink -f "$mux_prm_dir/current")
      local file_name
      file_name=$(basename "$link_file")
      printf "%s -> %s\n" "$(get_relative_path "$mux_prm_dir")/current" "$file_name"

      for file in "$mux_prm_dir"/*; do
        if [[ "$file" == "$link_file" || "$file" == "$mux_prm_dir/current" ]]; then
          continue
        fi
        printf "%s\n" "$(get_relative_path "$file")"
      done
    done
  done
}

main() {
  for tel_name in "${tel_names[@]}"; do
    for side_name in "${side_names[@]}"; do
      local mux_prm_dir="$ARTEMIS_WORKDIR/prm/$tel_name/pos_$side_name"
      check_dir "$mux_prm_dir" || continue

      rm -f "$mux_prm_dir/current"
      local options=()
      for file in "$mux_prm_dir"/*; do
        [[ "$(basename "$file")" != "current" ]] && options+=("$(basename "$file")")
      done

      if [ ${#options[@]} -eq 0 ]; then
        say "No files to link in $(get_relative_path "$mux_prm_dir"). Skipping..."
        continue
      fi

      local selected_file
      selected_file=$(printf "%s\n" "${options[@]}" | gum choose --height=10 --header="Select a file to link as 'current' in $(get_relative_path "$mux_prm_dir")")

      local relative_path
      relative_path=$(realpath --relative-to="$mux_prm_dir" "$mux_prm_dir/$selected_file")
      ln -sf "$relative_path" "$mux_prm_dir/current"
      say "Created symlink: $(get_relative_path "$mux_prm_dir")/current -> $selected_file"
    done
  done
}

need_cmd gum
need_cmd realpath

if [ -z $ARTEMIS_WORKDIR ]; then
  err 'Please set the "ARTEMIS_WORKDIR" environment variable and try again.'
fi

say "Select an operation:"
operation=$(gum choose "Set parameter" "Show list" "Clean parameters" "Exit")

case $operation in
"Set parameter")
  main
  ;;
"Show list")
  show_list
  ;;
"Clean parameters")
  prm_clean
  ;;
"Exit")
  say "Cancelled"
  exit 0
  ;;
esac
