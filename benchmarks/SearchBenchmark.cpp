#include <raze/algorithm/find/Search.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <algorithm>
#include <iostream>
#include <string>


template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmarkMain,
    SizeForBenchmark sizeForBenchmarkSub>
class StdSearchBenchmark {
public:
    static void SubAtBegin(benchmark::State& state) noexcept {
        _Char_* mainArr = new _Char_[sizeForBenchmarkMain];
        _Char_* subArr = new _Char_[sizeForBenchmarkSub];

        std::memset(mainArr, 1, sizeof(_Char_) * sizeForBenchmarkMain);
        std::memset(subArr, 2, sizeof(_Char_) * sizeForBenchmarkSub);

        const size_t copyCount = std::min<size_t>(sizeForBenchmarkMain, sizeForBenchmarkSub);
        for (size_t i = 0; i < copyCount; ++i)
            mainArr[i] = subArr[i];

        while (state.KeepRunning()) {
            auto* result = std::search(mainArr, mainArr + sizeForBenchmarkMain,
                subArr, subArr + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }

        delete[] mainArr;
        delete[] subArr;
    }

    static void SubInMiddle(benchmark::State& state) noexcept {
        _Char_* mainArr = new _Char_[sizeForBenchmarkMain];
        _Char_* subArr = new _Char_[sizeForBenchmarkSub];

        std::memset(mainArr, 1, sizeof(_Char_) * sizeForBenchmarkMain);
        std::memset(subArr, 2, sizeof(_Char_) * sizeForBenchmarkSub);

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain) {
            const size_t pos = sizeForBenchmarkMain / 2;
            for (size_t i = 0; i < sizeForBenchmarkSub; ++i)
                mainArr[pos + i] = subArr[i];
        }

        while (state.KeepRunning()) {
            auto* result = std::search(mainArr, mainArr + sizeForBenchmarkMain,
                subArr, subArr + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }

        delete[] mainArr;
        delete[] subArr;
    }

    static void SubAtEnd(benchmark::State& state) noexcept {
        _Char_* mainArr = new _Char_[sizeForBenchmarkMain];
        _Char_* subArr = new _Char_[sizeForBenchmarkSub];

        std::memset(mainArr, 1, sizeof(_Char_) * sizeForBenchmarkMain);
        std::memset(subArr, 2, sizeof(_Char_) * sizeForBenchmarkSub);

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain) {
            const size_t pos = sizeForBenchmarkMain - sizeForBenchmarkSub;
            for (size_t i = 0; i < sizeForBenchmarkSub; ++i)
                mainArr[pos + i] = subArr[i];
        }

        while (state.KeepRunning()) {
            auto* result = std::search(mainArr, mainArr + sizeForBenchmarkMain,
                subArr, subArr + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }

        delete[] mainArr;
        delete[] subArr;
    }
};


template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmarkMain,
    SizeForBenchmark sizeForBenchmarkSub>
class RazeSearchBenchmark {
public:
    static void SubAtBegin(benchmark::State& state) noexcept {
        _Char_* mainArr = new _Char_[sizeForBenchmarkMain];
        _Char_* subArr = new _Char_[sizeForBenchmarkSub];

        std::memset(mainArr, 1, sizeof(_Char_) * sizeForBenchmarkMain);
        std::memset(subArr, 2, sizeof(_Char_) * sizeForBenchmarkSub);

        const size_t copyCount = std::min<size_t>(sizeForBenchmarkMain, sizeForBenchmarkSub);
        for (size_t i = 0; i < copyCount; ++i)
            mainArr[i] = subArr[i];

        while (state.KeepRunning()) {
            auto* result = raze::algorithm::search(mainArr, mainArr + sizeForBenchmarkMain,
                subArr, subArr + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }

        delete[] mainArr;
        delete[] subArr;
    }

    static void SubInMiddle(benchmark::State& state) noexcept {
        _Char_* mainArr = new _Char_[sizeForBenchmarkMain];
        _Char_* subArr = new _Char_[sizeForBenchmarkSub];

        std::memset(mainArr, 1, sizeof(_Char_) * sizeForBenchmarkMain);
        std::memset(subArr, 2, sizeof(_Char_) * sizeForBenchmarkSub);

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain) {
            const size_t pos = sizeForBenchmarkMain / 2;
            for (size_t i = 0; i < sizeForBenchmarkSub; ++i)
                mainArr[pos + i] = subArr[i];
        }

        while (state.KeepRunning()) {
            auto* result = raze::algorithm::search(mainArr, mainArr + sizeForBenchmarkMain,
                subArr, subArr + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }

        delete[] mainArr;
        delete[] subArr;
    }

    static void SubAtEnd(benchmark::State& state) noexcept {
        _Char_* mainArr = new _Char_[sizeForBenchmarkMain];
        _Char_* subArr = new _Char_[sizeForBenchmarkSub];

        std::memset(mainArr, 1, sizeof(_Char_) * sizeForBenchmarkMain);
        std::memset(subArr, 2, sizeof(_Char_) * sizeForBenchmarkSub);

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain) {
            const size_t pos = sizeForBenchmarkMain - sizeForBenchmarkSub;
            for (size_t i = 0; i < sizeForBenchmarkSub; ++i)
                mainArr[pos + i] = subArr[i];
        }

        while (state.KeepRunning()) {
            auto* result = raze::algorithm::search(mainArr, mainArr + sizeForBenchmarkMain,
                subArr, subArr + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }

        delete[] mainArr;
        delete[] subArr;
    }
};


RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeSearchBenchmark, StdSearchBenchmark, raze::int8, SubAtBegin);
RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeSearchBenchmark, StdSearchBenchmark, raze::int16, SubAtBegin);
RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeSearchBenchmark, StdSearchBenchmark, raze::int32, SubAtBegin);
RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeSearchBenchmark, StdSearchBenchmark, raze::int64, SubAtBegin);

RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeSearchBenchmark, StdSearchBenchmark, raze::int8, SubInMiddle);
RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeSearchBenchmark, StdSearchBenchmark, raze::int16, SubInMiddle);
RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeSearchBenchmark, StdSearchBenchmark, raze::int32, SubInMiddle);
RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeSearchBenchmark, StdSearchBenchmark, raze::int64, SubInMiddle);

RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeSearchBenchmark, StdSearchBenchmark, raze::int8, SubAtEnd);
RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeSearchBenchmark, StdSearchBenchmark, raze::int16, SubAtEnd);
RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeSearchBenchmark, StdSearchBenchmark, raze::int32, SubAtEnd);
RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeSearchBenchmark, StdSearchBenchmark, raze::int64, SubAtEnd);

RAZE_BENCHMARK_MAIN();
