#include <raze/algorithm/order/ReverseCopy.h>
#include <algorithm>
#include <benchmarks/tools/BenchmarkHelper.h>

template <
    typename T,
    SizeForBenchmark sizeForBenchmark>
class StdReverseCopyBenchmark {
public:
    static inline auto src = FixedArray<T, sizeForBenchmark>{};
    static inline auto dest = FixedArray<T, sizeForBenchmark>{};

    static void ReverseCopy(benchmark::State& state) noexcept {
        while (state.KeepRunning()) {
            std::reverse_copy(src.data, src.data + sizeForBenchmark, dest.data);
            benchmark::DoNotOptimize(dest.data);
        }
    }
};

template <
    typename T,
    SizeForBenchmark sizeForBenchmark>
class RazeReverseCopyBenchmark {
public:
    static inline auto src = FixedArray<T, sizeForBenchmark>{};
    static inline auto dest = FixedArray<T, sizeForBenchmark>{};

    static void ReverseCopy(benchmark::State& state) noexcept {
        while (state.KeepRunning()) {
            raze::algorithm::reverse_copy(src.data, src.data + sizeForBenchmark, dest.data);
            benchmark::DoNotOptimize(dest.data);
        }
    }
};

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReverseCopyBenchmark, StdReverseCopyBenchmark, raze::int8, ReverseCopy);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReverseCopyBenchmark, StdReverseCopyBenchmark, raze::int16, ReverseCopy);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReverseCopyBenchmark, StdReverseCopyBenchmark, raze::int32, ReverseCopy);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReverseCopyBenchmark, StdReverseCopyBenchmark, raze::int64, ReverseCopy);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReverseCopyBenchmark, StdReverseCopyBenchmark, float, ReverseCopy);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReverseCopyBenchmark, StdReverseCopyBenchmark, double, ReverseCopy);

RAZE_BENCHMARK_MAIN();
