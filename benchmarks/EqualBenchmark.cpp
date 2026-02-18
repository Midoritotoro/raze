#include <raze/algorithm/find/Equal.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <uchar.h>
#include <wchar.h>
#include <cstring>
#include <vector>
#include <algorithm>

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmark>
class StdEqualBenchmark {
public:
    static void MismatchInEnd(benchmark::State& state) noexcept {
        std::vector<_Char_> array1(sizeForBenchmark);
        std::vector<_Char_> array2(sizeForBenchmark);

        std::memset(array1.data(), 0, sizeForBenchmark * sizeof(_Char_));
        std::memset(array2.data(), 0, sizeForBenchmark * sizeof(_Char_));

        array2[sizeForBenchmark - 1] = static_cast<_Char_>(42);

        while (state.KeepRunning()) {
            auto result = std::equal(array1.begin(), array1.end(), array2.begin());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void MismatchInMiddle(benchmark::State& state) noexcept {
        std::vector<_Char_> array1(sizeForBenchmark);
        std::vector<_Char_> array2(sizeForBenchmark);

        std::memset(array1.data(), 0, sizeForBenchmark * sizeof(_Char_));
        std::memset(array2.data(), 0, sizeForBenchmark * sizeof(_Char_));

        array2[sizeForBenchmark / 2] = static_cast<_Char_>(42);

        while (state.KeepRunning()) {
            auto result = std::equal(array1.begin(), array1.end(), array2.begin());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void MismatchInBegin(benchmark::State& state) noexcept {
        std::vector<_Char_> array1(sizeForBenchmark);
        std::vector<_Char_> array2(sizeForBenchmark);

        std::memset(array1.data(), 0, sizeForBenchmark * sizeof(_Char_));
        std::memset(array2.data(), 0, sizeForBenchmark * sizeof(_Char_));

        array2[0] = static_cast<_Char_>(42);

        while (state.KeepRunning()) {
            auto result = std::equal(array1.begin(), array1.end(), array2.begin());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmark>
class RazeEqualBenchmark {
public:
    static void MismatchInEnd(benchmark::State& state) noexcept {
        std::vector<_Char_> array1(sizeForBenchmark);
        std::vector<_Char_> array2(sizeForBenchmark);

        std::memset(array1.data(), 0, sizeForBenchmark * sizeof(_Char_));
        std::memset(array2.data(), 0, sizeForBenchmark * sizeof(_Char_));

        array2[sizeForBenchmark - 1] = static_cast<_Char_>(42);

        while (state.KeepRunning()) {
            auto result = raze::algorithm::equal(array1.begin(), array1.end(), array2.begin());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void MismatchInMiddle(benchmark::State& state) noexcept {
        std::vector<_Char_> array1(sizeForBenchmark);
        std::vector<_Char_> array2(sizeForBenchmark);

        std::memset(array1.data(), 0, sizeForBenchmark * sizeof(_Char_));
        std::memset(array2.data(), 0, sizeForBenchmark * sizeof(_Char_));

        array2[sizeForBenchmark / 2] = static_cast<_Char_>(42);

        while (state.KeepRunning()) {
            auto result = raze::algorithm::equal(array1.begin(), array1.end(), array2.begin());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void MismatchInBegin(benchmark::State& state) noexcept {
        std::vector<_Char_> array1(sizeForBenchmark);
        std::vector<_Char_> array2(sizeForBenchmark);

        std::memset(array1.data(), 0, sizeForBenchmark * sizeof(_Char_));
        std::memset(array2.data(), 0, sizeForBenchmark * sizeof(_Char_));

        array2[0] = static_cast<_Char_>(42);

        while (state.KeepRunning()) {
            auto result = raze::algorithm::equal(array1.begin(), array1.end(), array2.begin());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};


RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, raze::uint8, MismatchInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, raze::uint16, MismatchInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, raze::uint32, MismatchInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, raze::uint64, MismatchInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, float, MismatchInBegin);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, double, MismatchInBegin);

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, raze::uint8, MismatchInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, raze::uint16, MismatchInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, raze::uint32, MismatchInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, raze::uint64, MismatchInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, float, MismatchInMiddle);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, double, MismatchInMiddle);

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, raze::uint8, MismatchInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, raze::uint16, MismatchInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, raze::uint32, MismatchInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, raze::uint64, MismatchInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, float, MismatchInEnd);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeEqualBenchmark, StdEqualBenchmark, double, MismatchInEnd);

RAZE_BENCHMARK_MAIN();