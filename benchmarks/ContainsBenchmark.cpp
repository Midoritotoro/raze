#include <raze/algorithm/find/Contains.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <uchar.h>
#include <wchar.h> 

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmark>
class StdContainsBenchmark {
public:
    static void ContainsInEnd(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[sizeForBenchmark - 1] = 42;

        while (state.KeepRunning()) {
            auto result = std::ranges::contains(array, array + sizeForBenchmark, array[sizeForBenchmark - 1]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void ContainsInMiddle(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[sizeForBenchmark / 2] = 42;

        while (state.KeepRunning()) {
            auto result = std::ranges::contains(array, array + sizeForBenchmark, array[sizeForBenchmark / 2]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void ContainsInBegin(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[0] = 42;

        while (state.KeepRunning()) {
            auto result = std::ranges::contains(array, array + sizeForBenchmark, array[0]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmark>
class RazeContainsBenchmark {
public:
    static void ContainsInEnd(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[sizeForBenchmark - 1] = 42;

        while (state.KeepRunning()) {
            auto result = raze::algorithm::contains(array, array + sizeForBenchmark, array[sizeForBenchmark - 1]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void ContainsInMiddle(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[sizeForBenchmark / 2] = 42;

        while (state.KeepRunning()) {
            auto result = raze::algorithm::contains(array, array + sizeForBenchmark, array[sizeForBenchmark / 2]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void ContainsInBegin(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[0] = 42;

        while (state.KeepRunning()) {
            auto result = raze::algorithm::contains(array, array + sizeForBenchmark, array[0]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, raze::uint8, ContainsInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, raze::uint16, ContainsInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, raze::uint32, ContainsInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, raze::uint64, ContainsInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, float, ContainsInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, double, ContainsInBegin);

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, raze::uint8, ContainsInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, raze::uint16, ContainsInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, raze::uint32, ContainsInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, raze::uint64, ContainsInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, float, ContainsInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, double, ContainsInMiddle);

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, raze::uint8, ContainsInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, raze::uint16, ContainsInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, raze::uint32, ContainsInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, raze::uint64, ContainsInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, float, ContainsInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeContainsBenchmark, StdContainsBenchmark, double, ContainsInEnd);

RAZE_BENCHMARK_MAIN();