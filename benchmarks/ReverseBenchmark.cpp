#include <raze/algorithm/order/Reverse.h>
#include <algorithm>
#include <benchmarks/tools/BenchmarkHelper.h>

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmark>
class StdReverseBenchmark {
public:
    static inline auto array = FixedArray<_Char_, sizeForBenchmark>{};

    static void Reverse(benchmark::State& state) noexcept {
        while (state.KeepRunning()) {
            std::reverse(array.data, array.data + sizeForBenchmark);
            benchmark::DoNotOptimize(array.data);
        }
    }
};

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmark>
class RazeReverseBenchmark {
public:
    static inline auto array = FixedArray<_Char_, sizeForBenchmark>{};

    static void Reverse(benchmark::State& state) noexcept {

        while (state.KeepRunning()) {
            raze::algorithm::reverse(array.data, array.data + sizeForBenchmark);
            benchmark::DoNotOptimize(array.data);
        }
    }
};

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReverseBenchmark, StdReverseBenchmark, raze::int8, Reverse);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReverseBenchmark, StdReverseBenchmark, raze::int16, Reverse);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReverseBenchmark, StdReverseBenchmark, raze::int32, Reverse);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReverseBenchmark, StdReverseBenchmark, raze::int64, Reverse);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReverseBenchmark, StdReverseBenchmark, float, Reverse);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReverseBenchmark, StdReverseBenchmark, double, Reverse);


RAZE_BENCHMARK_MAIN();