# Building

## (Option 1) VSCode Extension

Install the `Remote - Containers` extenstion by Microsoft.
Open the project in VSCode.
Click the Dialog that says "Open in container".
Wait for VSCode to open in the container. This may take a few minutes the first time.

### VSCode Build Task
Once VSCode has started in the container, the project can be built using the predefined build task.

The build task can be run by pressing `Shift+Cmd+B` on a mac, or by pressing `Shift+Cmd+P` to open the commadn pallete and selecting "Tasks: Run Build Task".

### Manual Build
The project can also be built manually by running the following from the ASER-PTA directory.

```bash
mkdir build && cd build
cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang ..
```

In both cases executables will be placed in `ASER-PTA/build/bin`.


## (Option 2) Local Installation

Build LLVM-9.0.0 locally. Then run the following from the ASER-PTA root directory

```bash
# Set this to the directory containing the LLVMConfig.cmake file from your local LLVM-9.0 install
LLVM_DIR=/usr/local/Cellar/llvm//9.0.0_1/lib/cmake/llvm/ # Example used homebrew to install llvm
CXX=/usr/local/Cellar/llvm/9.0.0_1/bin/clang++
CC=/usr/local/Cellar/llvm/9.0.0_1/bin/clang

mkdir build && cd build
cmake \
    -DLLVM_DIR=$LLVM_DIR \
    -DCMAKE_C_COMPILER=$CC \
    -DCMAKE_CXX_COMPILER=$CXX \
    ..
make -j $(nproc)
```

### Building LLVM Locally

LLVM-9.0 can be built locally by running

```bash
git clone --depth 1 -b llvmorg-9.0.0 https://github.com/llvm/llvm-project.git
mkdir -p ./llvm-project/build
cd ./llvm-project/build
RUN cmake \
    -DLLVM_ENABLE_PROJECTS="clang" \
    -DLLVM_TARGETS_TO_BUILD="X86" \
    -DCMAKE_CXX_STANDARD="17" \
    -DLLVM_INCLUDE_EXAMPLES=OFF \
    -DLLVM_INCLUDE_TESTS=OFF \
    -DLLVM_INCLUDE_BENCHMARKS=OFF \
    -DLLVM_APPEND_VC_REV=OFF \
    -DLLVM_OPTIMIZED_TABLEGEN=ON \
    -DCMAKE_BUILD_TYPE=Release \
    ../llvm
make -j $(nproc)
```

# Running

To run the tool, simply type 
```console
./bin/racebench /your/path/to/the/IR/main.ll
```

> To get the IR file of the racebench
```console
cd /your/path/to/ASER-PTA/racebench
make
```
the main.ll file is the whole program llvm IR file

