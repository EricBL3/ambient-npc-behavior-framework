# Ambient Character Core Framework

A C++ implementation of a framework for creating realistic ambient character behavior.
This is part of my Master's project on ambient character behavior.

## 🚀 Quick Start

### Prerequisites
- **Git**
- **CMake 3.16+**
- **C++17 compatible compiler**
  - **Windows**: Visual Studio 2019+ or Visual Studio Build Tools
  - **macOS**: Xcode Command Line Tools (`xcode-select --install`)
  - **Linux**: GCC 8+ or Clang 7+

### Setup Instructions

#### 1. Clone Repository
```bash
git clone https://github.com/EricBL3/ambient-behavior-core-framework.git
cd AmbientCoreFramework
```

#### 2. Bootstrap vcpkg (first time only)
```
# Clone and bootstrap vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Windows
.\bootstrap-vcpkg.bat

# macOS/Linux
./bootstrap-vcpkg.sh

# Return to project root
cd ..
```

#### 3. Configure Project
```
# Windows
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows

# macOS
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-osx

# Linux
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-linux
```

#### 4. Build and Test
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
