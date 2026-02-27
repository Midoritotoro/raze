#include <raze/algorithm/find/FindEnd.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <random>

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmarkMain,
    SizeForBenchmark sizeForBenchmarkSub>
class StdFindEndBenchmark {
public:
    static void SubAtBegin(benchmark::State& state) noexcept {
        std::vector<_Char_> haystack(sizeForBenchmarkMain);
        std::vector<_Char_> needle(sizeForBenchmarkSub);

        std::fill(haystack.begin(), haystack.end(), static_cast<_Char_>(1));
        std::fill(needle.begin(), needle.end(), static_cast<_Char_>(2));

        std::copy(needle.begin(), needle.end(), haystack.begin());

        while (state.KeepRunning()) {
            auto result = std::find_end(haystack.begin(), haystack.end(), needle.begin(), needle.end());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }


    static void SubInMiddle(benchmark::State& state) noexcept {
        std::vector<_Char_> haystack(sizeForBenchmarkMain);
        std::vector<_Char_> needle(sizeForBenchmarkSub);

        std::fill(haystack.begin(), haystack.end(), static_cast<_Char_>(1));
        std::fill(needle.begin(), needle.end(), static_cast<_Char_>(2));

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain)
            std::copy(needle.begin(), needle.end(), haystack.begin() + (sizeForBenchmarkMain - sizeForBenchmarkSub) / 2);

        while (state.KeepRunning()) {
            auto result = std::find_end(haystack.begin(), haystack.end(), needle.begin(), needle.end());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }


    static void SubAtEnd(benchmark::State& state) noexcept {
        std::vector<_Char_> haystack(sizeForBenchmarkMain);
        std::vector<_Char_> needle(sizeForBenchmarkSub);

        std::fill(haystack.begin(), haystack.end(), static_cast<_Char_>(1));
        std::fill(needle.begin(), needle.end(), static_cast<_Char_>(2));

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain)
            std::copy(needle.begin(), needle.end(), haystack.begin() + (sizeForBenchmarkMain - sizeForBenchmarkSub));

        while (state.KeepRunning()) {
            auto result = std::find_end(haystack.begin(), haystack.end(), needle.begin(), needle.end());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmarkMain,
    SizeForBenchmark sizeForBenchmarkSub>
class RazeFindEndBenchmark {
public:
    static void SubAtBegin(benchmark::State& state) noexcept {
        std::vector<_Char_> haystack(sizeForBenchmarkMain);
        std::vector<_Char_> needle(sizeForBenchmarkSub);

        std::fill(haystack.begin(), haystack.end(), static_cast<_Char_>(1));
        std::fill(needle.begin(), needle.end(), static_cast<_Char_>(2));

        std::copy(needle.begin(), needle.end(), haystack.begin());

        while (state.KeepRunning()) {
            auto result = raze::algorithm::find_end(haystack.begin(), haystack.end(), needle.begin(), needle.end());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }


    static void SubInMiddle(benchmark::State& state) noexcept {
        std::vector<_Char_> haystack(sizeForBenchmarkMain);
        std::vector<_Char_> needle(sizeForBenchmarkSub);

        std::fill(haystack.begin(), haystack.end(), static_cast<_Char_>(1));
        std::fill(needle.begin(), needle.end(), static_cast<_Char_>(2));

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain)
            std::copy(needle.begin(), needle.end(), haystack.begin() + (sizeForBenchmarkMain - sizeForBenchmarkSub) / 2);

        while (state.KeepRunning()) {
            auto result = raze::algorithm::find_end(haystack.begin(), haystack.end(), needle.begin(), needle.end());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }


    static void SubAtEnd(benchmark::State& state) noexcept {
        std::vector<_Char_> haystack(sizeForBenchmarkMain);
        std::vector<_Char_> needle(sizeForBenchmarkSub);

        std::fill(haystack.begin(), haystack.end(), static_cast<_Char_>(1));
        std::fill(needle.begin(), needle.end(), static_cast<_Char_>(2));

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain)
            std::copy(needle.begin(), needle.end(), haystack.begin() + (sizeForBenchmarkMain - sizeForBenchmarkSub));

        while (state.KeepRunning()) {
            auto result = raze::algorithm::find_end(haystack.begin(), haystack.end(), needle.begin(), needle.end());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

#define REGISTER_ALL(TEST) \
    RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeFindEndBenchmark, StdFindEndBenchmark, raze::int8, TEST); \
    RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeFindEndBenchmark, StdFindEndBenchmark, raze::int16, TEST); \
    RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeFindEndBenchmark, StdFindEndBenchmark, raze::int32, TEST); \
    RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeFindEndBenchmark, StdFindEndBenchmark, raze::int64, TEST);

REGISTER_ALL(SubAtBegin)
REGISTER_ALL(SubInMiddle)
REGISTER_ALL(SubAtEnd)

RAZE_BENCHMARK_MAIN();
