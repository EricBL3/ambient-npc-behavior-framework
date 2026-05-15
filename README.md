# Ambient NPC Behavior Framework

A C++ framework for memory-driven ambient NPC behavior, implemented as a shared library for engine-agnostic integration. 
The framework selects actions for ambient NPCs using a memory-driven algorithm that produces behavioral variety without per-character scripting. 
It has been validated in both Unity and Unreal Engine.

Developed as part of a Master's thesis in Computer Science at Western University, supervised by Dr. Roberto Solis-Oba. 
An associated paper has been accepted at the IEEE Conference on Games 2026. 
Full documentation, video demos and sample projects are available at [https://www.csd.uwo.ca/~ebuitron/](https://www.csd.uwo.ca/~ebuitron/).

## Overview

Ambient NPCs in open-world games are expected to populate environments with varied, believable behavior across potentially hundreds of characters, while remaining within strict per-frame CPU budgets. This framework addresses that challenge using a memory-driven action selection algorithm: behaviors are defined as directed graphs, and a bounded memory mechanism biases selection toward untried or least-recently-used transitions, producing variety without per-character scripting or online planning.

The framework exposes a plain C API and loads behavior configurations from JSON files, making it independent of any specific engine or scripting system. Integration requires placing the compiled library in the engine's plugin or binary folder and writing a thin wrapper that calls the public C API. No other files from this repository are needed to use the framework.

## 🚀 Quick Start

### Prerequisites
- **Git**
- **CMake 3.16+**
- **C++20 compatible compiler**
  - **Windows**: Visual Studio 2022 (or 2019 v16.10+)
  - **macOS**: Xcode 13+ with Command Line Tools
    **Additionally (see macOS setup below): Homebrew, pkg-config**
  - **Linux**: GCC 10+ or Clang 12+
    
> **Note:** Precompiled releases are provided for Windows (x64) and macOS (arm64 / Apple Silicon only). Linux builds are supported from source using the instructions below.

### Setup Instructions

#### 1. Clone Repository
```bash
git clone https://github.com/EricBL3/ambient-npc-behavior-framework.git
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

## Citation

If you use this framework in your research, please cite the associated paper:

Eric Buitron-Lopez and Roberto Solis-Oba, "A Memory-Driven Action Selection Framework for Scalable Ambient NPC Behavior," to appear in *Proceedings of the 2026 IEEE Conference on Games (CoG)*, Madrid, Spain, September 1–4, 2026.

```bibtex
@inproceedings{buitronlopez2026,
    author    = {Buitron-Lopez, Eric and Solis-Oba, Roberto},
    title     = {A Memory-Driven Action Selection Framework for Scalable Ambient {NPC} Behavior},
    booktitle = {Proceedings of the 2026 IEEE Conference on Games (CoG)},
    year      = {2026},
    note      = {To appear}
}
```

This entry will be updated with final page numbers and DOI once the proceedings are published.

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
