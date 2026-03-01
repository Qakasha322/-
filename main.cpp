#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>

using Matrix = std::vector<std::vector<double>>;

struct RunInfo {
    std::string caseName;
    int n;
    long long operations;
    double ms;
    std::string verifyStatus;
};

static bool fileExists(const std::string& path) {
    std::ifstream f(path.c_str());
    return f.good();
}

static void ensureResultsDir() {
    CreateDirectoryA("results", NULL);
}

static std::vector<std::string> findAFilesInTestData() {
    std::vector<std::string> files;

    WIN32_FIND_DATAA data;
    HANDLE hFind = FindFirstFileA("test_data\\*_a.txt", &data);
    if (hFind == INVALID_HANDLE_VALUE) {
        return files;
    }

    do {
        if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            files.push_back(data.cFileName);
        }
    } while (FindNextFileA(hFind, &data));

    FindClose(hFind);
    std::sort(files.begin(), files.end());
    return files;
}

static bool readMatrix(const std::string& path, Matrix& matrix) {
    std::ifstream in(path.c_str());
    if (!in) {
        std::cerr << "Error: cannot open file " << path << "\n";
        return false;
    }

    int n = 0;
    if (!(in >> n) || n <= 0) {
        std::cerr << "Error: invalid matrix size in " << path << "\n";
        return false;
    }

    matrix.assign(n, std::vector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (!(in >> matrix[i][j])) {
                std::cerr << "Error: invalid matrix data in " << path << "\n";
                return false;
            }
        }
    }

    return true;
}

static bool writeMatrix(const std::string& path, const Matrix& matrix) {
    std::ofstream out(path.c_str());
    if (!out) {
        std::cerr << "Error: cannot write to file " << path << "\n";
        return false;
    }

    const int n = static_cast<int>(matrix.size());
    out << n << "\n";
    out << std::setprecision(15);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            out << matrix[i][j];
            if (j + 1 < n) {
                out << ' ';
            }
        }
        out << "\n";
    }

    return true;
}

static Matrix multiply(const Matrix& a, const Matrix& b) {
    const int n = static_cast<int>(a.size());
    Matrix c(n, std::vector<double>(n, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int k = 0; k < n; ++k) {
            for (int j = 0; j < n; ++j) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }

    return c;
}

static void writeCsvHeader(const std::string& csvPath) {
    std::ofstream out(csvPath.c_str(), std::ios::trunc);
    out << "case,n,operations,time_ms,mflops,verify\n";
}

static void appendCsv(const std::string& csvPath, const RunInfo& info) {
    const double mflops = (info.ms > 0.0) ? (static_cast<double>(info.operations) / (info.ms * 1000.0)) : 0.0;
    std::ofstream out(csvPath.c_str(), std::ios::app);
    out << info.caseName << ","
        << info.n << ","
        << info.operations << ","
        << std::fixed << std::setprecision(3) << info.ms << ","
        << std::fixed << std::setprecision(3) << mflops << ","
        << info.verifyStatus << "\n";
}

static int runPythonCommand(const std::string& scriptAndArgs) {
    const std::string cmdPython = "python " + scriptAndArgs;
    int code = std::system(cmdPython.c_str());
    if (code == 0) {
        return 0;
    }

    if (code == 9009) {
        const std::string cmdPyLauncher = "py -3 " + scriptAndArgs;
        return std::system(cmdPyLauncher.c_str());
    }

    return code;
}

static bool runSingleCase(const std::string& caseName,
                          const std::string& fileA,
                          const std::string& fileB,
                          const std::string& fileC,
                          RunInfo& info) {
    Matrix a;
    Matrix b;

    if (!readMatrix(fileA, a) || !readMatrix(fileB, b)) {
        return false;
    }

    if (a.size() != b.size()) {
        std::cerr << "Error: matrix sizes must match\n";
        return false;
    }

    const int n = static_cast<int>(a.size());
    const long long operations = 2LL * n * n * n;

    const auto t1 = std::chrono::high_resolution_clock::now();
    Matrix c = multiply(a, b);
    const auto t2 = std::chrono::high_resolution_clock::now();

    if (!writeMatrix(fileC, c)) {
        return false;
    }

    const double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

    std::cout << "Done: " << caseName << "\n";
    std::cout << "Size: " << n << "x" << n << "\n";
    std::cout << "Operations (2*N^3): " << operations << "\n";
    std::cout << "Time: " << std::fixed << std::setprecision(3) << ms << " ms\n";
    std::cout << "Result file: " << fileC << "\n";

    const std::string verifyArgs =
        "scripts/verify.py \"" + fileA + "\" \"" + fileB + "\" \"" + fileC + "\"";
    const int verifyCode = runPythonCommand(verifyArgs);

    std::string verifyStatus = "FAILED";
    if (verifyCode == 0) {
        verifyStatus = "OK";
    } else if (verifyCode == 9009) {
        verifyStatus = "SKIPPED";
        std::cerr << "Verification skipped: Python is not installed or not in PATH\n";
    } else {
        std::cerr << "Verification failed (exit code: " << verifyCode << ")\n";
    }

    info.caseName = caseName;
    info.n = n;
    info.operations = operations;
    info.ms = ms;
    info.verifyStatus = verifyStatus;
    return true;
}

int main(int argc, char* argv[]) {
    ensureResultsDir();
    const std::string csvPath = "results/timing.csv";

    if (argc == 1) {
        std::vector<std::string> aFiles = findAFilesInTestData();

        if (aFiles.empty()) {
            std::cerr << "Error: no files *_a.txt found in test_data\n";
            return 1;
        }

        writeCsvHeader(csvPath);

        int okCount = 0;
        int failCount = 0;

        for (size_t i = 0; i < aFiles.size(); ++i) {
            const std::string aName = aFiles[i];
            const std::string prefix = aName.substr(0, aName.size() - 6);
            const std::string fileA = "test_data/" + aName;
            const std::string fileB = "test_data/" + prefix + "_b.txt";
            const std::string fileC = "results/" + prefix + "_result.txt";

            if (!fileExists(fileB)) {
                std::cerr << "Skip: pair file not found for " << aName << "\n";
                ++failCount;
                continue;
            }

            RunInfo info = {"", 0, 0, 0.0, "SKIPPED"};
            if (runSingleCase(prefix, fileA, fileB, fileC, info)) {
                appendCsv(csvPath, info);
                ++okCount;
            } else {
                ++failCount;
            }
            std::cout << "----------------------------------------\n";
        }

        std::cout << "Finished. Success: " << okCount << ", Failed: " << failCount << "\n";
        const std::string visualizeArgs = "scripts/visualize.py \"" + csvPath + "\"";
        runPythonCommand(visualizeArgs);
        return failCount == 0 ? 0 : 1;
    }

    if (argc == 4) {
        writeCsvHeader(csvPath);
        RunInfo info = {"manual", 0, 0, 0.0, "SKIPPED"};
        bool ok = runSingleCase("manual", argv[1], argv[2], argv[3], info);
        if (ok) {
            appendCsv(csvPath, info);
            const std::string visualizeArgs = "scripts/visualize.py \"" + csvPath + "\"";
            runPythonCommand(visualizeArgs);
            return 0;
        }
        return 1;
    }

    std::cout << "Usage:\n";
    std::cout << "  matrix_mul                  (auto-run all pairs from test_data to results)\n";
    std::cout << "  matrix_mul A.txt B.txt C.txt\n";
    return 1;
}
