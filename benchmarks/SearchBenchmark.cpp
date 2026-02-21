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
        std::vector<_Char_> mainArr(sizeForBenchmarkMain);
        std::vector<_Char_> subArr(sizeForBenchmarkSub);

        std::fill(mainArr.begin(), mainArr.end(), static_cast<_Char_>(1));
        std::fill(subArr.begin(), subArr.end(), static_cast<_Char_>(2));

        const size_t copyCount = std::min<size_t>(sizeForBenchmarkMain, sizeForBenchmarkSub);
        for (size_t i = 0; i < copyCount; ++i)
            mainArr[i] = subArr[i];

        while (state.KeepRunning()) {
            auto* result = std::search(
                mainArr.data(), mainArr.data() + sizeForBenchmarkMain,
                subArr.data(), subArr.data() + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void SubInMiddle(benchmark::State& state) noexcept {
        std::vector<_Char_> mainArr(sizeForBenchmarkMain);
        std::vector<_Char_> subArr(sizeForBenchmarkSub);

        std::fill(mainArr.begin(), mainArr.end(), static_cast<_Char_>(1));
        std::fill(subArr.begin(), subArr.end(), static_cast<_Char_>(2));

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain) {
            const size_t pos = (sizeForBenchmarkMain - sizeForBenchmarkSub) / 2;
            for (size_t i = 0; i < sizeForBenchmarkSub; ++i)
                mainArr[pos + i] = subArr[i];
        }

        while (state.KeepRunning()) {
            auto* result = std::search(
                mainArr.data(), mainArr.data() + sizeForBenchmarkMain,
                subArr.data(), subArr.data() + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void SubAtEnd(benchmark::State& state) noexcept {
        std::vector<_Char_> mainArr(sizeForBenchmarkMain);
        std::vector<_Char_> subArr(sizeForBenchmarkSub);

        std::fill(mainArr.begin(), mainArr.end(), static_cast<_Char_>(1));
        std::fill(subArr.begin(), subArr.end(), static_cast<_Char_>(2));

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain) {
            const size_t pos = sizeForBenchmarkMain - sizeForBenchmarkSub;
            for (size_t i = 0; i < sizeForBenchmarkSub; ++i)
                mainArr[pos + i] = subArr[i];
        }

        while (state.KeepRunning()) {
            auto* result = std::search(
                mainArr.data(), mainArr.data() + sizeForBenchmarkMain,
                subArr.data(), subArr.data() + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};


template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmarkMain,
    SizeForBenchmark sizeForBenchmarkSub>
class RazeSearchBenchmark {
public:
    static void SubAtBegin(benchmark::State& state) noexcept {
        std::vector<_Char_> mainArr(sizeForBenchmarkMain);
        std::vector<_Char_> subArr(sizeForBenchmarkSub);

        std::fill(mainArr.begin(), mainArr.end(), static_cast<_Char_>(1));
        std::fill(subArr.begin(), subArr.end(), static_cast<_Char_>(2));

        const size_t copyCount = std::min<size_t>(sizeForBenchmarkMain, sizeForBenchmarkSub);
        for (size_t i = 0; i < copyCount; ++i)
            mainArr[i] = subArr[i];

        while (state.KeepRunning()) {
            auto* result = raze::algorithm::search(
                mainArr.data(), mainArr.data() + sizeForBenchmarkMain,
                subArr.data(), subArr.data() + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void SubInMiddle(benchmark::State& state) noexcept {
        std::vector<_Char_> mainArr(sizeForBenchmarkMain);
        std::vector<_Char_> subArr(sizeForBenchmarkSub);

        std::fill(mainArr.begin(), mainArr.end(), static_cast<_Char_>(1));
        std::fill(subArr.begin(), subArr.end(), static_cast<_Char_>(2));

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain) {
            const size_t pos = (sizeForBenchmarkMain - sizeForBenchmarkSub) / 2;
            for (size_t i = 0; i < sizeForBenchmarkSub; ++i)
                mainArr[pos + i] = subArr[i];
        }

        while (state.KeepRunning()) {
            auto* result = raze::algorithm::search(
                mainArr.data(), mainArr.data() + sizeForBenchmarkMain,
                subArr.data(), subArr.data() + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void SubAtEnd(benchmark::State& state) noexcept {
        std::vector<_Char_> mainArr(sizeForBenchmarkMain);
        std::vector<_Char_> subArr(sizeForBenchmarkSub);

        std::fill(mainArr.begin(), mainArr.end(), static_cast<_Char_>(1));
        std::fill(subArr.begin(), subArr.end(), static_cast<_Char_>(2));

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain) {
            const size_t pos = sizeForBenchmarkMain - sizeForBenchmarkSub;
            for (size_t i = 0; i < sizeForBenchmarkSub; ++i)
                mainArr[pos + i] = subArr[i];
        }

        while (state.KeepRunning()) {
            auto* result = raze::algorithm::search(
                mainArr.data(), mainArr.data() + sizeForBenchmarkMain,
                subArr.data(), subArr.data() + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
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
