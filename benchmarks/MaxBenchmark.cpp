#include <raze/algorithm/minmax/Max.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <ranges>
#include <utility>

template <
    typename _Type_,
    SizeForBenchmark sizeForBenchmark>
class StdMaxBenchmark {
public:
    static void MaxRange(benchmark::State& state) noexcept {
        _Type_ array[sizeForBenchmark];
        for (int i = 0; i < sizeForBenchmark; ++i)
            array[i] = i;

        while (state.KeepRunning()) {
            auto result = std::ranges::max(array);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

template <
    typename _Type_,
    SizeForBenchmark sizeForBenchmark>
class RazeMaxBenchmark {
public:
    static void MaxRange(benchmark::State& state) noexcept {
        _Type_ array[sizeForBenchmark];
        for (int i = 0; i < sizeForBenchmark; ++i)
            array[i] = i;

        while (state.KeepRunning()) {
            auto result = raze::algorithm::max_range(array, array + sizeForBenchmark);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMaxBenchmark, StdMaxBenchmark, raze::uint8, MaxRange);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMaxBenchmark, StdMaxBenchmark, raze::uint16, MaxRange);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMaxBenchmark, StdMaxBenchmark, raze::uint32, MaxRange);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMaxBenchmark, StdMaxBenchmark, raze::uint64, MaxRange);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMaxBenchmark, StdMaxBenchmark, float, MaxRange);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMaxBenchmark, StdMaxBenchmark, double, MaxRange);

RAZE_BENCHMARK_MAIN();
