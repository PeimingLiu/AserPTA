# Building

### Manual Build
The project can also be built manually by running the following from the ASER-PTA directory.

```bash
mkdir build && cd build
cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang ..
```

In both cases executables will be placed in `ASER-PTA/build/bin`.


## Local Installation

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
