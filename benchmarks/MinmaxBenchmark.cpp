#include <raze/algorithm/minmax/Minmax.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <ranges>
#include <utility>

template <
    typename _Type_,
    SizeForBenchmark sizeForBenchmark>
class StdMinmaxBenchmark {
public:
    static void MinmaxRange(benchmark::State& state) noexcept {
        _Type_ array[sizeForBenchmark];
        for (int i = 0; i < sizeForBenchmark; ++i)
            array[i] = i;

        while (state.KeepRunning()) {
            auto result = std::ranges::minmax(array);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

template <
    typename _Type_,
    SizeForBenchmark sizeForBenchmark>
class RazeMinmaxBenchmark {
public:
    static void MinmaxRange(benchmark::State& state) noexcept {
        _Type_ array[sizeForBenchmark];
        for (int i = 0; i < sizeForBenchmark; ++i)
            array[i] = i;

        while (state.KeepRunning()) {
            auto result = raze::algorithm::minmax_range(array, array + sizeForBenchmark);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMinmaxBenchmark, StdMinmaxBenchmark, raze::uint8, MinmaxRange);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMinmaxBenchmark, StdMinmaxBenchmark, raze::uint16, MinmaxRange);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMinmaxBenchmark, StdMinmaxBenchmark, raze::uint32, MinmaxRange);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMinmaxBenchmark, StdMinmaxBenchmark, raze::uint64, MinmaxRange);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMinmaxBenchmark, StdMinmaxBenchmark, float, MinmaxRange);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeMinmaxBenchmark, StdMinmaxBenchmark, double, MinmaxRange);

RAZE_BENCHMARK_MAIN();
