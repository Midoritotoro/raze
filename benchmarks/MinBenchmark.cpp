#include <raze/algorithm/minmax/Min.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <ranges>
#include <utility>

template <
    typename _Type_,
    SizeForBenchmark sizeForBenchmark>
class StdMinBenchmark {
public:
    static void MinRange(benchmark::State& state) noexcept {
        _Type_ array[sizeForBenchmark];
        for (int i = 0; i < sizeForBenchmark; ++i)
            array[i] = i;

        while (state.KeepRunning()) {
            auto result = std::ranges::min(array);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

template <
    typename _Type_,
    SizeForBenchmark sizeForBenchmark>
class RazeMinBenchmark {
public:
    static void MinRange(benchmark::State& state) noexcept {
        _Type_ array[sizeForBenchmark];
        for (int i = 0; i < sizeForBenchmark; ++i)
            array[i] = i;

        while (state.KeepRunning()) {
            auto result = raze::algorithm::min_range(array, array + sizeForBenchmark);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMinBenchmark, StdMinBenchmark, raze::uint8, MinRange);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMinBenchmark, StdMinBenchmark, raze::uint16, MinRange);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMinBenchmark, StdMinBenchmark, raze::uint32, MinRange);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMinBenchmark, StdMinBenchmark, raze::uint64, MinRange);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMinBenchmark, StdMinBenchmark, float, MinRange);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMinBenchmark, StdMinBenchmark, double, MinRange);

RAZE_BENCHMARK_MAIN();
