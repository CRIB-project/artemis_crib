# Python Environments (option)

By installing Python, you can use **pyROOT** and **TSrim** directly from Python.
However, Python is **not required** to use **artemis_crib**, so you may skip this section if you do not plan to use Python.

## Why Manage Python Environments?

Managing Python environments and dependencies is crucial to avoid compatibility issues.
Some configurations may work in specific environments but fail in others due to mismatched dependencies.
To address this, we recommend using tools that handle dependencies efficiently and isolate environments.

### Popular Tools for Python Environment Management

| Tool                                            | Description                                                                                                  |
| ----------------------------------------------- | ------------------------------------------------------------------------------------------------------------ |
| [**pyenv**](https://github.com/pyenv/pyenv)     | Manages multiple Python versions and switches between them on the same machine.                              |
| [**poetry**](https://python-poetry.org/)        | A dependency manager and build system for Python projects.                                                   |
| [**pipenv**](https://pipenv.pypa.io/en/latest/) | Combines `pip` and `virtualenv` for managing dependencies and virtual environments.                          |
| [**mise**](https://mise.jdx.dev/)[^note]        | Runtime manager (e.g., Python, Node.js, Java, Go). Ideal for multi-tool projects.                            |
| [**uv**](https://docs.astral.sh/uv/)            | A fast Python project manager (10-100x faster than `pip`), unifying tools like `pip`, `poetry`, and `pyenv`. |

[^note]:
    The author (okawak) uses a combination of `mise` and `uv` to manage Python environments.
    If your projects involve multiple tools, such as Python and Node.js, mise is highly effective for unified management.
    However, if you work exclusively with Python, `uv` is a simpler and more focused option.

## Using `uv` for a Global Python Environment

This section explains how to use **uv** to set up a global Python environment, required for tools like **pyROOT**.
**uv** typically creates project-specific virtual environments, but here we focus on configuring a global virtual environment.
For other methods, refer to the respective tool's documentation.

### Step 1: Install `uv`

Install `uv` using the following command:

```shell
curl -LsSf https://astral.sh/uv/install.sh | sh
```

Follow the instructions to complete the installation and configure your environment (e.g., adding `uv` to `PATH`).

Verify the installation:

```shell
uv --version
```

### Step 2: Install Python using `uv`

Install a specific Python version:

```shell
uv python install 3.12
```

- Replace `3.12` with the desired Python version.
- To view available versions:

  ```shell
  uv python list
  ```

<div class="warning">

Currently, Python installed via `uv` cannot be globally accessed via the `python` command.
This feature is expected in future releases.
For now, use `uv venv` to create a global virtual environment.

</div>

### Step 3: Create a Global Virtual Environment

To create a global virtual environment:

```shell
cd $HOME
uv venv
```

This creates a `.venv` directory in `$HOME`.

### Step 4: Persist the Environment Activation

Edit your shell configuration file to activate the virtual environment at startup:

```shell
vi $HOME/.zshrc
```

Add:

```bash
# Activate the global uv virtual environment
if [[ -d "$HOME/.venv" ]]; then
    source "$HOME/.venv/bin/activate"
fi
```

Apply the changes:

```shell
source $HOME/.zshrc
```

Verify the Python executable:

```shell
which python
```

Ensure the output is `.venv/bin/python`.

### Step 5: Add Common Packages

Install commonly used Python packages into the virtual environment:

```shell
uv pip install numpy pandas
```

## Additional information

- For more detail, refer to the [uv documentation](https://docs.astral.sh/uv/).
