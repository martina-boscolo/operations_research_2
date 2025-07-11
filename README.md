# Algorithms for Traveling Salesman Problem

This repository contains the project completed for the Operations Research 2 course. 
The project focuses on investigating various strategies for solving the well-known Travelling Salesman Problem (TSP), from heuristic techniques that provide approximate solutions to exact optimization using the CPLEX MIP solver.

### Development Environment

The project has been developed and tested with the following tools on Windows + VSCode:

- gcc (MinGW\.org GCC-6.3.0-1) 6.3.0
- GNU Make 3.82.90
- gnuplot 6.0 patchlevel 2
- IBM(R) ILOG(R) CPLEX(R) Interactive Optimizer 22.1.2.0


### Build instructions using CMake

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

### Run from Release directory

```
cd Release
tsp.exe [options]
```

## Usage and Command Line Parameters

```
./Release/tsp.exe -method <method> -n <nodes> -seed <seed> -timelimit <seconds> -verbose <level> -param1 <value1> -param2 <value2> -param3 <value3>
```

### Input/Output Options

- **`-file|-f <file_path>`** - Specify input file containing TSP instance data
- **`-n <number>`** - Set number of nodes for random instance generation (minimum required)

### General Configuration

- **`-seed <number>`** - Set random seed for reproducible results
- **`-timelimit <seconds>`** - Set time limit in seconds 
- **`-verbose <level>`** - Set verbosity level for debug output

### Solution Methods

Use `-method <method_name>` to specify the solving algorithm:

#### Heuristic Methods

- **`NN`** - Nearest Neighbor algorithm
  - `-param1 1` - Enable 2-opt refinement
- **`MS_NN`** - Multi-start Nearest Neighbor
  - `-param1 1` - Enable 2-opt refinement
- **`EM`** - Extra Mileage heuristic
  - `-param1 1` - Enable 2-opt refinement

#### Metaheuristic Methods

- **`VNS`** - Variable Neighborhood Search
  - `-param1 <k_value>` - Neighborhood size parameter (3, 5)
  - `-param2 <r_value>` - Search intensity parameter (>= 1)
- **`TS`** - Tabu Search
  - `-param1 <tenure_type>` - Tenure strategy (0-4):
    - `0` - Fixed minimum tenure
    - `1` - Fixed maximum tenure
    - `2` - Random tenure with spikes
    - `3` - Linear (sawtooth) pattern
    - `4` - Sinusoidal pattern

#### Exact Methods

- **`BL`** - Benders' loop
- **`BC`** - Branch and Cut
  - `-param1 <0|1>` - Enable warm-up heuristic (0=disabled, 1=enabled)
  - `-param2 <0|1>` - Enable fractional SECs (0=disabled, 1=enabled)
  - `-param3 <0|1>` - Enable post-optimization heuristic (0=disabled, 1=enabled)

#### Matheuristic Methods

- **`HF`** - Hard Fixing
  - `-param1 <strategy>` - Fixing strategy:
    - `0` - Random percentage selection among (40, 50, 60, 80)
    - `>1` - Specified percentage
- **`LB`** - Local Branching
  - `-param1 <k_value>` - Neighborhood size
    - `0` - Fix 2% of nodes;
    - `>1` - Use specified percentage

#### Examples

```
./Release/tsp.exe -method MS_NN -n 1000  -timelimit 60 -verbose 30
```
```
./Release/tsp.exe -method TS -n 500 -seed 15 -param1 4 -timelimit 90
```
```
./Release/tsp.exe -method HF -n 1000 -seed 20 -param1 60 -timelimit 180
```
