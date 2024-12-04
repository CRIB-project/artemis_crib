# Make New Users

This section explains how to create working directories for individual users.
With the overall structure now prepared, you are ready to start using `artemis_crib`!

## Steps

### 1. Run `artlogin`

To add a new user (working directory), use the `artlogin` command.
If no arguments are provided, the directory corresponding to the `EXP_NAME` specified in `.zshrc` or `.bashrc` will be created.

```shell
artlogin username
```

- If the `username` directory already exists, the necessary environment variables will be loaded, and you will be moved to that directory.
- If it does not exist, you will be prompted to enter configuration details interactively.

> **Note:** The `artlogin2` command is also available.
> Unlike `artlogin`, `artlogin2` uses `EXP_NAME_OLD` as the environment variable.
> This is useful for analyzing data from a past experiment while keeping the current experiment name in `EXP_NAME`.
> Replace `artlogin` with `artlogin2` as needed.

### 2. Interactive Configuration

When creating a new user, you will be prompted as follows.
If you ran the command by mistake, type `n` to exit.

```plaintext
Create new user? (y/n):
```

If you type `y`, the setup continues.

Next, you will be asked to provide your name and email address.
This information is used by Git to track changes made by the user:

```plaintext
Input full name:
Input email address:
```

The repository will then be cloned, and symbolic links (e.g., `ridf`, `output`, `rootfile`) specified during the `artnew` command setup will be created.
You will automatically move to the new working directory.

### 3. Build the Source Code

The CRIB-related source code is located in the working directory and must be built before use.
Follow the standard CMake build process:

```shell
mkdir build
cd build
cmake ..
make -j4  # Adjust the number of cores as needed
make install
cd ..
```

When running `cmake`, a `thisartemis-crib.sh` file will be created in the working directory.
This file is used to load environment variables.
While the `artlogin` command loads it automatically, for the initial setup, run the following commands manually or rerun `artlogin`:

```shell
artlogin username
```

or

```shell
source thisartemis-crib.sh
```

## Useful Commands

### `acd`

The `acd` command is an alias defined after running `artlogin`.
It allows you to quickly navigate to the working directory.

```bash
acd='cd ${ARTEMIS_WORKDIR}'
```

### `a`

The `a` command launches the `artemis` interpreter.
It only works in directories containing the `artemislogon.C` file and is defined in the `art_setting` shell script.

Example implementation:

```bash
a() {
  # Check if 'artemislogon.C' exists in the current directory
  if [ ! -e "artemislogon.C" ]; then
    printf "\033[1ma\033[0m: 'artemislogon.C' not found\n"
    return 1
  fi

  # Determine if the user is connected via SSH and adjust DISPLAY accordingly
  if [ -z "${SSH_CONNECTION:-}" ]; then
    # Not connected via SSH
    artemis -l "$@"
  elif [ -f ".vncdisplay" ]; then
    # Connected via SSH and .vncdisplay exists
    DISPLAY=":$(cat .vncdisplay)" artemis -l "$@"
  else
    # Connected via SSH without .vncdisplay
    artemis -l "$@"
  fi
}
```
