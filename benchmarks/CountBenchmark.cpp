#include <raze/algorithm/find/Count.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <uchar.h>
#include <wchar.h>

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmark>
class StdCountBenchmark {
public:
    static void CountAllEqual(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];

        for (int i = 0; i < sizeForBenchmark; ++i)
            array[i] = 42;

        while (state.KeepRunning()) {
            auto result = std::count(array, array + sizeForBenchmark, 42);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void CountEverySecond(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];

        for (int i = 0; i < sizeForBenchmark; ++i)
            if (i % 2 == 0)
                array[i] = 42;
            else
                array[i] = 0;

        while (state.KeepRunning()) {
            auto result = std::count(array, array + sizeForBenchmark, 42);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void CountEveryFourth(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];

        for (int i = 0; i < sizeForBenchmark; ++i)
            if (i % 4 == 0)
                array[i] = 42;
            else
                array[i] = 0;

        while (state.KeepRunning()) {
            auto result = std::count(array, array + sizeForBenchmark, 42);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmark>
class RazeCountBenchmark {
public:
    static void CountAllEqual(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];

        for (int i = 0; i < sizeForBenchmark; ++i)
            array[i] = 42;

        while (state.KeepRunning()) {
            auto result = raze::algorithm::count(array, array + sizeForBenchmark, 42);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void CountEverySecond(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];

        for (int i = 0; i < sizeForBenchmark; ++i)
            if (i % 2 == 0)
                array[i] = 42;
            else
                array[i] = 0;

        while (state.KeepRunning()) {
            auto result = raze::algorithm::count(array, array + sizeForBenchmark, 42);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void CountEveryFourth(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];

        for (int i = 0; i < sizeForBenchmark; ++i)
            if (i % 4 == 0)
                array[i] = 42;
            else
                array[i] = 0;

        while (state.KeepRunning()) {
            auto result = raze::algorithm::count(array, array + sizeForBenchmark, 42);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, raze::uint8, CountAllEqual);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, raze::uint16, CountAllEqual);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, raze::uint32, CountAllEqual);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, raze::uint64, CountAllEqual);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, float, CountAllEqual);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, double, CountAllEqual);

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, raze::uint8, CountEverySecond);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, raze::uint16, CountEverySecond);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, raze::uint32, CountEverySecond);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, raze::uint64, CountEverySecond);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, float, CountEverySecond);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, double, CountEverySecond);

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, raze::uint8, CountEveryFourth);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, raze::uint16, CountEveryFourth);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, raze::uint32, CountEveryFourth);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, raze::uint64, CountEveryFourth);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, float, CountEveryFourth);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeCountBenchmark, StdCountBenchmark, double, CountEveryFourth);

RAZE_BENCHMARK_MAIN();