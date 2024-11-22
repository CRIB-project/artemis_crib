# Artemis

This section provides detailed instructions for installing **artemis**, which serves as the foundation for **artemis_crib**.
While **artemis_crib** is specifically customized for experiments performed at CRIB, **artemis** is a general-purpose analysis framework.

## Steps to Install artemis

1. Navigate to the directory where you want to install **artemis**:

   ```bash
   cd /path/to/installation
   ```

2. Clone the artemis repository:

   ```bash
   git clone https://github.com/artemis-dev/artemis.git
   cd artemis
   ```

3. Switch to the develop branch:
   The `develop` branch is compatible with ROOT version 6 and is recommended for installation.

   ```bash
   git switch develop
   ```

4. Create a build directory and configure the build:
   You can customize the build with the following options:

   ```bash
   mkdir build
   cd build
   cmake -DCMAKE_INSTALL_PREFIX=<installdir> ..
   ```

   **CMake Configuration Options**

   | Option                   | Default Value | Description                                                                                                                                                                                   |
   | ------------------------ | ------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
   | `-DCMAKE_INSTALL_PREFIX` | `./install`   | Specifies the installation directory. Replace `<installdir>` with your desired directory.                                                                                                     |
   | `-DBUILD_GET`            | `OFF`         | Enables or disables building the GET decoder. If `ON`, specify the GET decoder path using `-DWITH_GET_DECODER`.                                                                               |
   | `-DWITH_GET_DECODER`     | Not Set       | Specifies the path to the GET decoder. Required when `-DBUILD_GET=ON`.                                                                                                                        |
   | `-DCMAKE_PREFIX_PATH`    | Not Set       | Specifies paths to `yaml-cpp` or `openMPI`. If not found automatically, you must set it manually. Note that `yaml-cpp` is required, but MPI support will be disabled if `openMPI` is missing. |
   | `-DBUILD_WITH_REDIS`     | `OFF`         | Enables or disables Redis integration.                                                                                                                                                        |
   | `-DBUILD_WITH_ZMQ`       | `OFF`         | Enables or disables ZeroMQ integration.                                                                                                                                                       |

   **Example: Customized Configuration Command**

   ```bash
   cmake -DCMAKE_INSTALL_PREFIX=/path/to/installation -DBUILD_GET=ON -DWITH_GET_DECODER=/path/to/decoder -DBUILD_WITH_REDIS=ON -DBUILD_WITH_ZMQ=ON ..
   ```

5. Compile and install artemis:

   ```bash
   cmake --build . --target install -- -j4
   ```

   - Adjust the `-j` option based on your system's CPU cores (e.g., `-j8` for 8 cores).

6. Set up the artemis environment:
   After installation, a script named `thisartemis.sh` will be generated in the installation directory. Run the following command to set up the environment variables:

   ```bash
   source <installdir>/bin/thisartemis.sh
   ```

## Persisting the Environment Setup

To avoid running the `source` command manually every time, add it to your shell configuration file (e.g., `.zshrc` or `.bashrc`):

```bash
echo 'source <installdir>/bin/thisartemis.sh' >> ~/.zshrc
source ~/.zshrc
```

This ensures that the artemis environment is automatically loaded when a new shell session starts.

## Further Information

- For additional details about artemis, visit the [artemis GitHub repository](https://github.com/artemis-dev/artemis/tree/develop).
