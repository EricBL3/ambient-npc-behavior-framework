# Ambient Character Core Framework

A C++ framework for memory-driven ambient NPC behavior, implemented as a Dynamic Link Library (DLL) for engine-agnostic integration. 
The framework selects actions for ambient NPCs using a memory-driven algorithm that produces behavioral variety without per-character scripting. 
It has been validated in both Unity and Unreal Engine.

Developed as part of a Master's thesis in Computer Science at Western University, supervised by Dr. Roberto Solis-Oba. 
An associated paper has been accepted at the IEEE Conference on Games 2026. 
Full documentation, build prerequisites, and integration guides are available at [https://www.csd.uwo.ca/~ebuitron/](https://www.csd.uwo.ca/~ebuitron/).

## 🚀 Quick Start

### Prerequisites
- **Git**
- **CMake 3.16+**
- **C++20 compatible compiler**
  - **Windows**: Visual Studio 2022 (or 2019 v16.10+)
  - **macOS**: Xcode 13+ with Command Line Tools
    **Additionally (see macOS setup below): Homebrew, pkg-config**
  - **Linux**: GCC 10+ or Clang 12+

### Setup Instructions

#### 1. Clone Repository
```bash
git clone repo-link (https or ssh)
cd AmbientCoreFramework
```

#### 2. macOS Additional Setup (skip if you're on Windows/Linux)
```
# If you don't have Homebrew installed, install it using:
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install the required build tools
brew install pkg-config

# Make sure Xcode Command Line Tools are installed:
xcode-select --install

```

#### 3. Bootstrap vcpkg (first time only)
```
# Initialize and bootstrap vcpkg
git submodule update --init --recursive
cd vcpkg

# Windows
.\bootstrap-vcpkg.bat

# macOS/Linux
./bootstrap-vcpkg.sh

# Return to project root
cd ..
```

#### 4. Install dependencies via vcpkg
```
cd vcpkg

# Install dependencies listed in vcpkg.json manifest
./vcpkg install

cd ..
```

#### 5. Configure Project
```
# Windows
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows

# macOS (Intel)
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-osx

#macOS (Apple Silicon / ARM)
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=arm64-osx

# Linux
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-linux
```

#### 6. Build and Test
```
# Build the project
cmake --build build --config Release

# Run tests
cd build
ctest --config Release
cd ..
```

## 🛠️ Development Commands
```
# After making changes to source files
cmake --build build

# After making changes to CMakeLists.txt
cmake -S . -B build
cmake --build build

# Run tests
cd build && ctest && cd ..

# Run specific test
cd build && ctest -R "TestName" && cd ..

# Clean build
rm -rf build  # or rmdir /s build on Windows
# Then reconfigure with platform-specific command from step 3

# Build and test in one command
cmake --build build && cd build && ctest && cd ..

# Verbose build (see compilation commands)
cmake --build build --verbose

# Debug build
cmake --build build --config Debug

# Run tests with verbose output
cd build && ctest --verbose && cd ..
```

## Usage

The framework is initialized with three engine-provided callbacks, loaded from four JSON configuration files, and driven by a single update call per game loop tick.

```cpp
FrameworkHandle handle = CreateAmbientBehaviorFramework(envConditionCallback, startActionCallback, queryPositionCallback);
InitializeAmbientBehaviorFramework(handle, schemePath, sequencesPath, actionsPath, conditionsPath, logConfig);

// Per tick:
Update(handle, batchSize, currentTime);

// Shutdown:
ShutdownAmbientBehaviorFramework(handle);
```

For full integration instructions, configuration file reference, and engine-specific setup guides, see the [project webpage](https://www.csd.uwo.ca/~ebuitron/).

## Generating Documentation
1. Install Doxygen in your system.
2. Run `doxygen Doxyfile` in the project root (AmbientCoreFramework folder).
3. Open the documentation with one of the following commands
  ```
  open docs/doxygen/html/index.html   # macOS
  xdg-open docs/doxygen/html/index.html # Linux
  start docs\doxygen\html\index.html   # Windows (PowerShell or CMD)
  ```

## License

This project is licensed under the Apache License 2.0. See [LICENSE](LICENSE) for details.
