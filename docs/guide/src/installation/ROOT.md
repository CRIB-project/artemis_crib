# ROOT

The **artemis** and **artemis_crib** tools are built based on the ROOT library.
Before installing these tools, you must install ROOT on your system.

## Why Build ROOT from Source?

Since **artemis** and **artemis_crib** may depend on a specific version of ROOT, it is recommended to build ROOT from source rather than using a package manager.
This approach ensures compatibility and access to all required features.

## Steps to Build ROOT from Source

1. Navigate to the directory where you want to install ROOT:

   ```bash
   cd /path/to/installation
   ```

2. Clone the ROOT repository:

   ```bash
   git clone https://github.com/root-project/root.git root_src
   ```

3. Checkout the desired version (replace `<branch name>` and `<tag name>` with the specific version):

   ```bash
   cd root_src
   git switch -c <branch name> <tag name>
   cd ..
   ```

4. Create build and installation directories, and configure the build:

   ```bash
   mkdir <builddir> <installdir>
   cd <builddir>
   cmake -DCMAKE_INSTALL_PREFIX=<installdir> -Dmathmore=ON ../root_src
   ```

   - Set `mathmore` to `ON` because artemis relies on this library for advanced mathematical features.

5. Compile and install ROOT:

   ```bash
   cmake --build . --target install -- -j4
   ```

   - Adjust the `-j` option based on the number of CPU cores available (e.g., `-j8` for 8 cores) to optimize the build process.

6. Set up the ROOT environment:

   ```bash
   source <installdir>/bin/thisroot.sh
   ```

   - Replace `<installdir>` with the actual installation directory.
   - Running this command loads the necessary environment variables for ROOT.

## Persisting the Environment Setup

To avoid running the `source` command manually each time, add it to your shell configuration file (e.g., .zshrc or .bashrc):

```bash
echo 'source <installdir>/bin/thisroot.sh' >> ~/.zshrc
source ~/.zshrc
```

This ensures that the ROOT environment is automatically loaded whenever a new shell session starts.

## Important Note for pyROOT Users

If you plan to use **pyROOT**, make sure your Python environment is set up before proceeding with the ROOT installation.
Refer to the [Python Setting section](./python.md) for detailed instructions on setting up Python and managing virtual environments.

## Additional information

- For more details and troubleshooting, consult the official [ROOT installation guide](https://root.cern/install/).
- Ensure your system meets all prerequisites listed in the ROOT documentation, including necessary libraries and tools.
- Manage ROOT versions appropriately to maintain compatibility with your analysis environment and dependent tools.
