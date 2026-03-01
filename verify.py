#!/usr/bin/env python3
import sys


def read_matrix(path, np):
    with open(path, "r", encoding="utf-8") as f:
        n_line = f.readline().strip()
        if not n_line:
            raise ValueError(f"empty file: {path}")

        n = int(n_line)
        if n <= 0:
            raise ValueError(f"invalid size in {path}: {n}")

        rows = []
        for i in range(n):
            line = f.readline()
            if not line:
                raise ValueError(f"not enough rows in {path}")
            values = [float(x) for x in line.split()]
            if len(values) != n:
                raise ValueError(f"row {i + 1} in {path} has wrong length")
            rows.append(values)

    return np.array(rows, dtype=np.float64)


def main():
    if len(sys.argv) != 4:
        print("Usage: python scripts/verify.py A.txt B.txt C.txt")
        return 2

    try:
        import numpy as np
    except Exception as e:
        print(f"Verification error: NumPy is not available ({e})")
        return 2

    a_path, b_path, c_path = sys.argv[1], sys.argv[2], sys.argv[3]

    try:
        a = read_matrix(a_path, np)
        b = read_matrix(b_path, np)
        c = read_matrix(c_path, np)
    except Exception as e:
        print(f"Verification error: {e}")
        return 1

    try:
        expected = a @ b
    except Exception as e:
        print(f"Verification error: cannot multiply A and B ({e})")
        return 1

    if c.shape != expected.shape:
        print(f"Verification: FAILED")
        print(f"Shape mismatch: result {c.shape}, expected {expected.shape}")
        return 1

    if np.allclose(c, expected, atol=1e-9, rtol=0.0):
        print("Verification: OK")
        return 0

    diff = np.abs(c - expected)
    max_diff = float(np.max(diff))
    max_idx = np.unravel_index(np.argmax(diff), diff.shape)
    print("Verification: FAILED")
    print(f"Max difference: {max_diff}")
    print(f"At index: {max_idx}")
    print(f"Expected value: {expected[max_idx]}")
    print(f"Actual value: {c[max_idx]}")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
