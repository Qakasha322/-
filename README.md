# Matrix Multiplication (Simple Student Version)

Simple C++ program for multiplying two square matrices.

## What the program does
- Reads two square matrices from files.
- Multiplies them.
- Saves result matrix to file.
- Prints matrix size, operation count (`2*N^3`) and runtime.
- Runs automatic verification with Python + NumPy.
- Saves benchmark table to `results/timing.csv`.
- Builds PNG chart with matplotlib: `results/performance.png`.

## Matrix file format

```text
N
a11 a12 ... a1N
...
aN1 aN2 ... aNN
```

Example (`2x2`):

```text
2
1 2
3 4
```

## Build (no CMake)

### Windows (MSVC, Developer Command Prompt)

```cmd
cl /EHsc /O2 /std:c++17 main.cpp /Fe:matrix_mul.exe
```

### Windows (MinGW g++)

```powershell
g++ -O2 -std=c++17 main.cpp -o matrix_mul.exe
```

### Linux/macOS

```bash
g++ -O2 -std=c++17 main.cpp -o matrix_mul
```

## Run

### 1) Automatic mode (recommended)

Runs all pairs from `test_data` (`*_a.txt` + `*_b.txt`), writes matrix results to `results/`.

```cmd
matrix_mul.exe
```

### 2) Manual mode

```cmd
matrix_mul.exe A.txt B.txt C.txt
```

## Python dependencies

Install once:

```cmd
py -3 -m pip install numpy matplotlib
```

## Verification and visualization

Program automatically runs:
- `scripts/verify.py` for each case
- `scripts/visualize.py results/timing.csv` after run

Generated files:
- `results/<case>_result.txt`
- `results/timing.csv`
- `results/performance.png`

Verification status in CSV:
- `OK` - result is correct
- `FAILED` - result mismatch or input error
- `SKIPPED` - Python is not available in PATH
