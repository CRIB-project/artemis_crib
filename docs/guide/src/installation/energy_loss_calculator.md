# Energy Loss Calculator

**TSrim** is a ROOT-based library, derived from `TF1`, designed to calculate the range or energy loss of ions in materials using SRIM range data.
Unlike directly interpolating SRIM's output files, **TSrim** fits a polynomial function to the log(Energy) vs. log(Range) data for specific ion-target pairs, ensuring high performance and accuracy.

At CRIB, tools like [**enewz**](https://github.com/CRIB-project/enewz) and [**SRIMlib**](https://github.com/CRIB-project/SRIMlib) have also been developed for energy loss calculations.
Among them, **TSrim**, developed by S. Hayakawa, stands out for its versatility and is supported in **artemis_crib**.

## Prerequisites

- **C++17 or later**: Required for compilation.
- **ROOT installed**: Ensure ROOT is installed and accessible in your environment.
- **Python 3.9 or later** (optional): For Python integration.

## Steps to Build with CMake

### 1. Clone the Repository

Navigate to the desired installation directory and clone the repository:

```shell
cd /path/to/installation
git clone https://github.com/CRIB-project/TSrim.git
cd TSrim
```

To use this library with Python, clone the python_develop branch:

```shell
git switch python_develop
```

### Configure the Build

Create a build directory and configure the build with CMake.
You can specify a custom installation directory using `-DCMAKE_INSTALL_PREFIX`:

```shell
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=../install ..
```

If no directory is specified, the default installation path is `/usr/local`.

### Compile and Install

Build and install the library:

```shell
cmake --build . --target install -- -j4
```

- Adjust the `-j` option based on the number of CPU cores available (e.g., `-j8` for 8 cores) to speed up the build process.

## Uninstallation

To remove TSrim, run one of the following commands from the build directory:

```shell
make uninstall
```

or

```shell
cmake --build . --target uninstall
```

## Usage in Other CMake Projects

TSrim supports CMake's `find_package` functionality. To link TSrim to your project, add the following to your `CMakeLists.txt`:

```plaintext
find_package(TSrim REQUIRED)
target_link_libraries(${TARGET_NAME} TSrim::Srim)
```

### Additional Resources

- [TSrim GitHub Repository](https://github.com/CRIB-project/TSrim)
- [SRIM on ROOT Slide (Google Slide)](https://docs.google.com/presentation/d/1v2fcSzfREJnktkHS7z6tXroHQbBpR1BSlsRj4ryszQc/edit#slide=id.p)
