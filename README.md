# Operations Research 2

### Build instructions
To build the project, use the following command:
```mingw32-make```.

This will compile the project and generate the output executable.

### Development Environment
The project has been developed and tested with the following tools on Windows + VSCode:
- gcc (MinGW\.org GCC-6.3.0-1) 6.3.0
- GNU Make 3.82.90
- gnuplot 6.0 patchlevel 2

You can verify the versions of each tool by running the following commands:
```
gcc --version
mingw32-make --version
gnuplot --version
```
### Build with CMake
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```
### Run with Cmake
```
cd Release
tsp.exe ...... (as before)
```