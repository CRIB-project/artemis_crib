# Requirements

This page outlines the packages required to run **artemis_crib**, including ROOT.
Since most of the code is based on ROOT, please refer to the official [ROOT dependencies](https://root.cern/install/dependencies/) for more information.

## Compilers

- **C++17** support is required
- **gcc 8** or later is supported
- (currently Clang is not supported)

## Required packages

The following is an example command to install the packages typically used on an **Ubuntu-based** distribution:

```bash
sudo apt install binutils cmake dpkg-dev g++ gcc libssl-dev git libx11-dev \
    libxext-dev libxft-dev libxpm-dev libtbb-dev libvdt-dev libgif-dev libyaml-cpp-dev \
    htop tmux vim emacs wget curl build-essential
```

- The first set of packages (e.g., `binutils`, `cmake`, `g++`) includes essential tools and libraries required to compile and run the code.
- `libyaml-cpp-dev`: A YAML parser library used for configuration and data input.
- `htop`, `tmux`, `vim`, `emacs`, `wget`, `curl`: Optional but commonly used tools for system monitoring, session management, and file downloads, making the environment more convenient for server-side analysis.
- `build-essential`: A meta-package that ensures essential compilation tools are installed.

<div class="warning">

To use **pyROOT** or the Python interface of **TSrim**, Python must be installed.
Although Python can be installed using a package manager like `apt` (`sudo apt install python3 python3-pip python3-venv`), it is recommended to use tools such as **pyenv** to create a virtual environment.
Popular tools for managing Python environments include:

- [pyenv](https://github.com/pyenv/pyenv) + [poetry](https://python-poetry.org/) or [pipenv](https://github.com/pypa/pipenv)
- [mise](https://mise.jdx.dev/)
- [uv](https://docs.astral.sh/uv/)

If you plan to use **pyROOT**, ensure that the Python environment is fully set up before proceeding to the next section.
Instructions for setting up the Python environment are available in the [Python Setting section](./python.md).

</div>

## CRIB analysis machine specifications

The functionality of "artemis_crib" has been confirmed in the following environment:

- Ubuntu 22.04.4 (LTS)
- gcc 11.4.0
- cmake 3.22.1
- ROOT 6.30/04
- yaml-cpp 0.7
- artemis commit ID a976bb9
