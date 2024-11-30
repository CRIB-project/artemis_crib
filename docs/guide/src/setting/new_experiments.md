# Make New Experiments

This guide explains how to set up the environment for a new experiment.
At CRIB, we use a shared user for experiments and organize them within the `art_analysis` directory.
The typical directory structure looks like this[^note]:

```plaintext
~/art_analysis
├── exp_name1
│   ├── exp_name1 # default (shared) user
│   ...
│   └── okawak # individual user
├── exp_name2
│   ├── exp_name2 # default (shared) user
│   ...
│   └── okawak # individual user
├── bin/
├── .conf/
│   ├── exp_name1.sh
│   ├── exp_name2.sh
│   ...
```

[^note]:
    Different organizations may follow their own conventions.
    At CRIB, this directory structure is assumed.

## Steps to Set Up a New Experiment

### 1. Start Setup with `artnew`

Run the following command to begin the setup process:

```shell
artnew
```

This command will guide you interactively through the configuration process.

### 2. Input Experimental Name

When prompted:

```plaintext
Input experimental name:
```

Enter a **unique name** for your experiment.
This name will be used to create directories and configuration files.
Choose something meaningful to identify the experiment.

### 3. Input Base Repository Path or URL

Next, you will see:

```plaintext
Input base repository path or URL:
```

Specify the Git repository for **artemis_crib** or your custom source.
By default, the GitHub repository is cloned to create a new analysis environment.
If you’ve prepared a different working directory, enter its path.

<div class="warning">

Note: CRIB servers support SSH cloning.
For personal environments without SSH key registration, use HTTPS.

</div>

### 4. Input Raw Data Directory Path

Provide the path where your experiment’s binary data (e.g., `.ridf` files) is stored:

```plaintext
Input rawdata directory path:
```

The system creates a symbolic link named `ridf` in the working directory, pointing to the specified path.
If needed, you can adjust this link manually after setup.

### 5. Input Output Data Directory Path

Next, specify the directory for storing output data:

```plaintext
Input output data directory path:
```

A symbolic link named `output` will point to this directory.
If you prefer to store files directly in the output directory of your working environment,
you can manually modify the configuration after setup.

### 6. Choose Repository Setup Option

Finally, decide how to manage the Git repository:

```plaintext
Select an option for repository setup:
  1. Create a new repository locally.
  2. Clone default branch and create a new branch.
  3. Use the repository as is. (for developers)
```

- **Option 1**: Creates a local repository. Use this if all work will remain local.
- **Option 2**: Clones the default branch from GitHub and creates a new branch for the experiment.
- **Option 3**: Uses the main branch as-is. This option is recommended for developers.

## Verifying the Configuration

After completing the setup, the configuration file will be saved in `art_analysis/.conf/exp_name.sh`.
Update your shell configuration to include the experiment name:

```shell
vi .zshrc # or .bashrc
```

Add the following line:

```bash
export EXP_NAME="exp_name"
```

Reload the shell configuration:

```shell
source .zshrc
```

### Using `artlogin`

Run the following command to set up the working directories ([next section](./new_users.md)):

```shell
artlogin
```

If you encounter the following error:

```plaintext
artlogin: Environment for 'exp_name' not found. Create it using 'artnew' command.
```

Check the following:

- Ensure `art_analysis/.conf/exp_name.sh` was created successfully.
- Verify that `EXP_NAME` in your shell configuration (`.zshrc` or `.bashrc`) is correct and loaded.
