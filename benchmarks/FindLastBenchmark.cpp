#include <raze/algorithm/find/FindLast.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <uchar.h>
#include <wchar.h>

#include <ranges>


template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmark>
class StdFindLastBenchmark {
public:
    static void FindInEnd(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[sizeForBenchmark - 1] = 42;

        while (state.KeepRunning()) {
            auto result = std::ranges::find_last(array, array + sizeForBenchmark, array[sizeForBenchmark - 1]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void FindInMiddle(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[sizeForBenchmark / 2] = 42;

        while (state.KeepRunning()) {
            auto result = std::ranges::find_last(array, array + sizeForBenchmark, array[sizeForBenchmark / 2]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void FindInBegin(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[0] = 42;

        while (state.KeepRunning()) {
            auto result = std::ranges::find_last(array, array + sizeForBenchmark, array[0]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmark>
class RazeFindLastBenchmark {
public:
    static void FindInEnd(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[sizeForBenchmark - 1] = 42;

        while (state.KeepRunning()) {
            auto* result = raze::algorithm::find_last(array, array + sizeForBenchmark, array[sizeForBenchmark - 1]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void FindInMiddle(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[sizeForBenchmark / 2] = 42;

        while (state.KeepRunning()) {
            auto* result = raze::algorithm::find_last(array, array + sizeForBenchmark, array[sizeForBenchmark / 2]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void FindInBegin(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        std::memset(array, 0, sizeof(array));

        array[0] = 42;

        while (state.KeepRunning()) {
            auto* result = raze::algorithm::find_last(array, array + sizeForBenchmark, array[0]);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, raze::uint8, FindInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, raze::uint16, FindInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, raze::uint32, FindInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, raze::uint64, FindInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, float, FindInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, double, FindInBegin);

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, raze::uint8, FindInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, raze::uint16, FindInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, raze::uint32, FindInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, raze::uint64, FindInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, float, FindInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, double, FindInMiddle);

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, raze::uint8, FindInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, raze::uint16, FindInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, raze::uint32, FindInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, raze::uint64, FindInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, float, FindInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFindLastBenchmark, StdFindLastBenchmark, double, FindInEnd);

RAZE_BENCHMARK_MAIN();