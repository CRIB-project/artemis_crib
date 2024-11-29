# Art_analysis

When using **artemis**, it is customary to create a directory named `art_analysis` in your `$HOME` directory to organize and perform all analysis tasks.
This section explains how to set up the `art_analysis` directory structure and configure the required shell scripts.

## Initialize the Directory Structure

Run the following command to create the directory structure and download the necessary shell scripts:

```bash
curl -fsSL --proto '=https' --tlsv1.2 https://crib-project.github.io/artemis_crib/scripts/init.sh | sh
```

This script will:

1. Create the `art_analysis` directory in `$HOME` if it does not already exist.
2. Set up subdirectories and shell scripts in `art_analysis/bin`.
3. Automatically assign the appropriate permissions to all scripts.

If the `art_analysis` directory already exists, the script will make no changes.

## Directory Structure Overview

After running the script, the `art_analysis` directory will be organized as follows:

```plaintext
art_analysis/
├── bin/
│   ├── art_setting
│   ├── artnew
│   ├── artup
├── .conf/  (empty directory)
│   ├── artlogin.sh
```

- `bin/`: Contains shell scripts used for various analysis tasks.
- `.conf/`: Reserved for configuration files.

## Configuring the PATH and Loading `art_setting`

To use the scripts in `art_analysis/bin` globally, add the directory to your `PATH` environment variable.

1. Edit your shell configuration file (e.g., `.bashrc` or `.zshrc`) and add the following line:

   ```bash
   export PATH="$HOME/art_analysis/bin:$PATH"
   ```

2. Apply the changes:

   ```shell
   source ~/.zshrc  # or source ~/.bashrc
   ```

3. Verify the configuration:

   ```shell
   which art_setting
   ```

   The output should point to `~/art_analysis/bin/art_setting`.

### Automatically Loading `art_setting`

The `art_setting` script defines several functions to simplify analysis tasks using **artemis**.
To make these functions available in every shell session, add the following line to your shell configuration file:

```bash
source $HOME/art_analysis/bin/art_setting
```

Apply the changes:

```shell
source ~/.zshrc  # or source ~/.bashrc
```

## Overview of Scripts

The following scripts are included in `art_analysis/bin`:

- `art_setting`: Sets up functions for the analysis environment.
- `artnew`: Creates directories and files for new analysis sessions.
- `artup`: Updates the shell scripts and settings.
- `artlogin.sh`: Configures individual analysis environments and automatically loads environment variables.

## Example Shell Configuration (e.g., .zshrc)

Below is an example of a complete `.zshrc` configuration file.
It includes all the settings required for artemis and related tools, ensuring proper initialization in each shell session.

```bash
# Activate the global uv virtual environment
if [[ -d "$HOME/.venv" ]]; then
    source "$HOME/.venv/bin/activate"
fi

# artemis configuration
if [[ -d "$HOME/art_analysis" ]]; then
    # ROOT
    source <root_installdir>/bin/thisroot.sh >/dev/null 2>&1

    # TSrim (if needed)
    source <tsrim_installdir>/bin/thisTSrim.sh >/dev/null 2>&1

    # artemis
    source <artemis_installdir>/install/bin/thisartemis.sh >/dev/null 2>&1
    export EXP_NAME="exp_name"
    export EXP_NAME_OLD="exp_old_name"

    # Add art_analysis/bin to PATH
    export PATH="$HOME/art_analysis/bin:$PATH"

    # Load artemis functions
    source "$HOME/art_analysis/bin/art_setting"
fi
```

### Notes

- Replace `<root_installdir>`, `<tsrim_installdir>`, and `<artemis_installdir>` with the actual paths on your system.
- Set appropriate values for `EXP_NAME` and `EXP_NAME_OLD` based on your experiment settings. These are explained in the next section: [Make New Experiment](../setting/new_experiments.md).
