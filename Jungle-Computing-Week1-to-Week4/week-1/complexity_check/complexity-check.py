import time
import tracemalloc
import argparse
import matplotlib.pyplot as plt

# from selection_sort import gen_rnd_list
# from insertion_sort import gen_rnd_list
from bubble_sort import gen_rnd_list

def measure(func, input_sizes):
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

    times, mem_mib, mem_kib = measure(gen_rnd_list, input_sizes)
    plot_results(input_sizes, times, mem_mib, mem_kib)


if __name__ == "__main__":
    main()
