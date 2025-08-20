#!/usr/bin/env python3
"""
Script to measure the time complexity and memory usage of a given function
Usage:
    python measure_complexity.py

Define your target function in this file (e.g., `eratosthenes(n)`) and it will be measured
for input sizes [10000, 20000, 40000, 80000, 160000, 320000].
The script reports execution time and peak memory usage (in MiB and KiB), and plots both metrics.
"""
import time  # for high-resolution timing (docs.python.org)
import tracemalloc  # for memory allocation tracing (docs.python.org)
import argparse  # for CLI interface (docs.python.org)
import matplotlib.pyplot as plt  # for plotting results (matplotlib.org)

# from quick_sort import quick_sort_gen_rnd_list
# from radix_sort import radix_sort_gen_rnd_list
# from quick_sort_inplace import quick_sort_gen_rnd_list
# from heap_sort import heap_sort_gen_rnd_list 
from merge_sort import merge_sort_gen_rnd_list

def measure(func, input_sizes):
    """
    Measure execution time and peak memory of `func(n)` for each n in input_sizes.
    Returns three lists: times (seconds), memories_mib (MiB), memories_kib (KiB).
    """
    times = []
    memories_mib = []
    memories_kib = []
    for n in input_sizes:
        tracemalloc.start()
        start = time.perf_counter()
        func(n)
        end = time.perf_counter()
        current, peak = tracemalloc.get_traced_memory()
        tracemalloc.stop()

        elapsed = end - start
        peak_mib = peak / 1024 / 1024
        peak_kib = peak / 1024
        times.append(elapsed)
        memories_mib.append(peak_mib)
        memories_kib.append(peak_kib)
        print(f"n={n}: time={elapsed:.6f}s, memory={peak_mib:.6f} MiB ({peak_kib:.2f} KiB)")
    return times, memories_mib, memories_kib


def plot_results(input_sizes, times, memories_mib, memories_kib):
    """
    Plot time vs. input size, memory (MiB) vs. input size, and memory (KiB) vs. input size.
    Saves figures to files and displays them.
    """
    # Time complexity plot
    plt.figure()
    plt.plot(input_sizes, times, marker='o')
    plt.title("Time vs Input Size")
    plt.xlabel("Input Size (n)")
    plt.ylabel("Time (seconds)")
    plt.grid(True)
    plt.savefig("time_complexity.png")
    plt.show()

    # Memory usage plot (MiB)
    plt.figure()
    plt.plot(input_sizes, memories_mib, marker='o')
    plt.title("Memory Usage vs Input Size (MiB)")
    plt.xlabel("Input Size (n)")
    plt.ylabel("Memory (MiB)")
    plt.grid(True)
    plt.savefig("memory_usage_mib.png")
    plt.show()

    # Memory usage plot (KiB)
    plt.figure()
    plt.plot(input_sizes, memories_kib, marker='o')
    plt.title("Memory Usage vs Input Size (KiB)")
    plt.xlabel("Input Size (n)")
    plt.ylabel("Memory (KiB)")
    plt.grid(True)
    plt.savefig("memory_usage_kib.png")
    plt.show()


def main():
    parser = argparse.ArgumentParser(
        description="Measure time and memory complexity of a function for varying input sizes"
    )
    parser.add_argument(
        "--sizes", type=int, nargs='+',
        default=[10000, 20000, 40000, 80000, 160000, 320000],
        help="List of input sizes to test (default: typical sequence)"
    )
    args = parser.parse_args()
    input_sizes = args.sizes

    # TODO: Replace or modify this function with your own target algorithm
    # def eratosthenes(n):
    #     sieve = [True] * (n + 1)
    #     for p in range(2, int(n**0.5) + 1):
    #         if sieve[p]:
    #             for i in range(p*p, n + 1, p):
    #                 sieve[i] = False
    #     return [i for i, prime in enumerate(sieve) if prime and i >= 2]

    times, mem_mib, mem_kib = measure(merge_sort_gen_rnd_list, input_sizes)
    plot_results(input_sizes, times, mem_mib, mem_kib)


if __name__ == "__main__":
    main()

