
#include <raze/algorithm/remove/Remove.h>
#include <benchmarks/tools/BenchmarkHelper.h> 

#include <algorithm>
#include <vector>
#include <numeric>
#include <cstddef>


template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmark>
class StdRemoveBenchmark {
public:
    static void RemoveNone(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        _Char_ value_to_remove = static_cast<_Char_>(sizeForBenchmark + 1);

        while (state.KeepRunning()) {
            for (int i = 0; i < sizeForBenchmark; ++i) {
                array[i] = static_cast<_Char_>(i % (sizeForBenchmark > 0 ? sizeForBenchmark : 1));
            }

            auto* new_end = std::remove(array, array + sizeForBenchmark, value_to_remove);
            benchmark::DoNotOptimize(new_end);
            benchmark::ClobberMemory();
        }
    }

    static void RemoveHalf(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        _Char_ value_to_remove = static_cast<_Char_>(0);

        while (state.KeepRunning()) {
            for (int i = 0; i < sizeForBenchmark; ++i) {
                array[i] = static_cast<_Char_>(i % 2);
            }

            auto* new_end = std::remove(array, array + sizeForBenchmark, value_to_remove);
            benchmark::DoNotOptimize(new_end);
            benchmark::ClobberMemory();
        }
    }

    static void RemoveAll(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        _Char_ value_to_remove = static_cast<_Char_>(42);

        while (state.KeepRunning()) {
            for (int i = 0; i < sizeForBenchmark; ++i) {
                array[i] = value_to_remove;
            }

            auto* new_end = std::remove(array, array + sizeForBenchmark, value_to_remove);
            benchmark::DoNotOptimize(new_end);
            benchmark::ClobberMemory();
        }
    }
};

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmark>
class RazeRemoveBenchmark {
public:
    static void RemoveNone(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        _Char_ value_to_remove = static_cast<_Char_>(sizeForBenchmark + 1);

        while (state.KeepRunning()) {
            for (int i = 0; i < sizeForBenchmark; ++i) {
                array[i] = static_cast<_Char_>(i % (sizeForBenchmark > 0 ? sizeForBenchmark : 1));
            }

            auto* new_end = raze::algorithm::remove(array, array + sizeForBenchmark, value_to_remove);
            benchmark::DoNotOptimize(new_end);
            benchmark::ClobberMemory();
        }
    }

    static void RemoveHalf(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        _Char_ value_to_remove = static_cast<_Char_>(0);

        while (state.KeepRunning()) {
            for (int i = 0; i < sizeForBenchmark; ++i) {
                array[i] = static_cast<_Char_>(i % 2);
            }

            auto* new_end = raze::algorithm::remove(array, array + sizeForBenchmark, value_to_remove);
            benchmark::DoNotOptimize(new_end);
            benchmark::ClobberMemory();
        }
    }

    static void RemoveAll(benchmark::State& state) noexcept {
        _Char_ array[sizeForBenchmark];
        _Char_ value_to_remove = static_cast<_Char_>(42);

        while (state.KeepRunning()) {
            for (int i = 0; i < sizeForBenchmark; ++i) {
                array[i] = value_to_remove;
            }

            auto* new_end = raze::algorithm::remove(array, array + sizeForBenchmark, value_to_remove);
            benchmark::DoNotOptimize(new_end);
            benchmark::ClobberMemory();
        }
    }
};

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, raze::uint8, RemoveNone);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, raze::uint16, RemoveNone);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, raze::uint32, RemoveNone);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, raze::uint64, RemoveNone);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, float, RemoveNone);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, double, RemoveNone);

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, raze::uint8, RemoveHalf);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, raze::uint16, RemoveHalf);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, raze::uint32, RemoveHalf);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, raze::uint64, RemoveHalf);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, float, RemoveHalf);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, double, RemoveHalf);

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, raze::uint8, RemoveAll);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, raze::uint16, RemoveAll);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, raze::uint32, RemoveAll);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, raze::uint64, RemoveAll);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, float, RemoveAll);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeRemoveBenchmark, StdRemoveBenchmark, double, RemoveAll);

RAZE_BENCHMARK_MAIN();