# eokas
Eokas is an open source programming language that is similar to C++ but makes it easy to build simple, reliable, and efficient software.

## Build

### Prerequsites
* C++ Compilers which support C++20 standard.
* CMake 3.20.x or later.

### System Environment Variables
|ENV                  |Comment                           |Reference                                                               |
|---------------------|----------------------------------|------------------------------------------------------------------------|
|LLVM_SDK_PATH        |LLVM root path                    |https://llvm.org                                                        |

### Build Steps
* On Linux and macOS, please execute build.sh in terminal.
```shell
./build.sh
```

* On Windows, please execute the bat script in command line.
```shell
./build.bat
```

## CLIs of eokas
```shell
# Run eokas source file in JIT engine.
eokas run --file test.eokas

# Compile eokas source file to object
eokas compile --file test.eokas
```

## License
MIT license

## Reference
* [Building LLVM with CMake](https://llvm.org/docs/CMake.html)
* 

<!--

## Acknowledgements
[JetBrains](https://jb.gg/OpenSource) 

-->

