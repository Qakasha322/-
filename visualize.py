#!/usr/bin/env python3
import csv
import os
import sys


def load_rows(csv_path):
    rows = []
    with open(csv_path, "r", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(
                {
                    "case": row["case"],
                    "n": int(row["n"]),
                    "time_ms": float(row["time_ms"]),
                    "mflops": float(row["mflops"]),
                    "verify": row["verify"],
                }
            )
    return rows


def main():
    if len(sys.argv) < 2 or len(sys.argv) > 3:
        print("Usage: python scripts/visualize.py results/timing.csv [--show]")
        return 1

    csv_path = sys.argv[1]
    show_plot = len(sys.argv) == 3 and sys.argv[2] == "--show"

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except Exception as e:
        print(f"Visualization error: matplotlib is not available ({e})")
        return 2

    try:
        rows = load_rows(csv_path)
    except Exception as e:
        print(f"Visualization error: cannot read CSV ({e})")
        return 1

    if not rows:
        print("Visualization: no data")
        return 0

    cases = [r["case"] for r in rows]
    times = [r["time_ms"] for r in rows]
    speeds = [r["mflops"] for r in rows]

    colors = []
    for r in rows:
        if r["verify"] == "OK":
            colors.append("#2ca02c")
        elif r["verify"] == "SKIPPED":
            colors.append("#ff7f0e")
        else:
            colors.append("#d62728")

    fig, axes = plt.subplots(2, 1, figsize=(11, 8), constrained_layout=True)

    axes[0].bar(cases, times, color=colors)
    axes[0].set_title("Matrix Multiplication Time")
    axes[0].set_ylabel("Time (ms)")
    axes[0].tick_params(axis="x", rotation=25)

    axes[1].bar(cases, speeds, color=colors)
    axes[1].set_title("Matrix Multiplication Speed")
    axes[1].set_ylabel("MFLOPS")
    axes[1].set_xlabel("Case")
    axes[1].tick_params(axis="x", rotation=25)

    legend_handles = [
        plt.Rectangle((0, 0), 1, 1, color="#2ca02c", label="OK"),
        plt.Rectangle((0, 0), 1, 1, color="#d62728", label="FAILED"),
        plt.Rectangle((0, 0), 1, 1, color="#ff7f0e", label="SKIPPED"),
    ]
    axes[0].legend(handles=legend_handles, loc="upper right")

    output_dir = os.path.dirname(csv_path) or "."
    output_path = os.path.join(output_dir, "performance.png")
    fig.savefig(output_path, dpi=150)

    print(f"Visualization saved: {output_path}")

    if show_plot:
        # Note: Agg backend usually does not open GUI windows.
        # PNG is always saved; use this flag only if your backend supports GUI.
        try:
            plt.show()
        except Exception:
            pass

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
