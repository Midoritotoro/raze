#include <raze/algorithm/find/Find.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <uchar.h>
#include <wchar.h> 

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmark>
class StdFindBenchmark {
public:
    static void FindInEnd(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[sizeForBenchmark - 1] = 42;

        while (state.KeepRunning()) {
            auto* result = std::find(array, array + sizeForBenchmark, array[sizeForBenchmark - 1]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void FindInMiddle(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[sizeForBenchmark / 2] = 42;

        while (state.KeepRunning()) {
            auto* result = std::find(array, array + sizeForBenchmark, array[sizeForBenchmark / 2]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void FindInBegin(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[0] = 42;

        while (state.KeepRunning()) {
            auto* result = std::find(array, array + sizeForBenchmark, array[0]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmark>
class RazeFindBenchmark {
public:
    static void FindInEnd(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[sizeForBenchmark - 1] = 42;

        while (state.KeepRunning()) {
            auto* result = raze::algorithm::find(array, array + sizeForBenchmark, array[sizeForBenchmark - 1]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void FindInMiddle(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[sizeForBenchmark / 2] = 42;

        while (state.KeepRunning()) {
            auto* result = raze::algorithm::find(array, array + sizeForBenchmark, array[sizeForBenchmark / 2]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void FindInBegin(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[0] = 42;

        while (state.KeepRunning()) {
            auto* result = raze::algorithm::find(array, array + sizeForBenchmark, array[0]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, raze::uint8, FindInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, raze::uint16, FindInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, raze::uint32, FindInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, raze::uint64, FindInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, float, FindInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, double, FindInBegin);

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, raze::uint8, FindInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, raze::uint16, FindInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, raze::uint32, FindInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, raze::uint64, FindInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, float, FindInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, double, FindInMiddle);

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, raze::uint8, FindInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, raze::uint16, FindInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, raze::uint32, FindInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, raze::uint64, FindInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, float, FindInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindBenchmark, StdFindBenchmark, double, FindInEnd);

RAZE_BENCHMARK_MAIN();