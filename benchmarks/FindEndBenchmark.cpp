#include <raze/algorithm/find/FindEnd.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <random>

template <class T>
static void fill_random(std::vector<T>& v, int seed = 12345) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 255);
    for (auto& x : v) x = static_cast<T>(dist(rng));
}

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmarkMain,
    SizeForBenchmark sizeForBenchmarkSub>
class StdFindEndBenchmark {
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
            auto* result = std::find_end(
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
            auto* result = std::find_end(
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
            auto* result = std::find_end(
                mainArr.data(), mainArr.data() + sizeForBenchmarkMain,
                subArr.data(), subArr.data() + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void SubWithPartialRepeats(benchmark::State& state) noexcept {
        std::vector<_Char_> mainArr(sizeForBenchmarkMain);
        std::vector<_Char_> subArr(sizeForBenchmarkSub);

        std::fill(subArr.begin(), subArr.end(), static_cast<_Char_>(2));
        std::fill(mainArr.begin(), mainArr.end(), static_cast<_Char_>(1));

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain) {
            const size_t pos = sizeForBenchmarkMain / 3;

            for (size_t i = 0; i < sizeForBenchmarkSub / 2; ++i)
                mainArr[pos + i] = subArr[i];

            for (size_t i = sizeForBenchmarkSub / 2; i < sizeForBenchmarkSub; ++i)
                mainArr[pos + i] = static_cast<_Char_>(3);
        }

        while (state.KeepRunning()) {
            auto* result = std::find_end(
                mainArr.data(), mainArr.data() + sizeForBenchmarkMain,
                subArr.data(), subArr.data() + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void SubWithManyFalsePositives(benchmark::State& state) noexcept {
        std::vector<_Char_> mainArr(sizeForBenchmarkMain);
        std::vector<_Char_> subArr(sizeForBenchmarkSub);

        for (size_t i = 0; i < sizeForBenchmarkSub; ++i)
            subArr[i] = static_cast<_Char_>(5 + i * 2);

        for (size_t i = 0; i < sizeForBenchmarkMain; ++i) {
            if (i % 3 == 0)
                mainArr[i] = subArr[0];
            else if (i % 5 == 0)
                mainArr[i] = subArr[sizeForBenchmarkSub - 1];
            else
                mainArr[i] = static_cast<_Char_>(100 + (i % 50));
        }

        while (state.KeepRunning()) {
            auto* result = std::find_end(
                mainArr.data(), mainArr.data() + sizeForBenchmarkMain,
                subArr.data(), subArr.data() + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void RandomData(benchmark::State& state) noexcept {
        std::vector<_Char_> mainArr(sizeForBenchmarkMain);
        std::vector<_Char_> subArr(sizeForBenchmarkSub);

        fill_random(mainArr);
        fill_random(subArr);

        while (state.KeepRunning()) {
            auto* result = std::find_end(
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
class RazeFindEndBenchmark {
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
            auto* result = raze::algorithm::find_end(
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
            auto* result = raze::algorithm::find_end(
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
            auto* result = raze::algorithm::find_end(
                mainArr.data(), mainArr.data() + sizeForBenchmarkMain,
                subArr.data(), subArr.data() + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void SubWithPartialRepeats(benchmark::State& state) noexcept {
        std::vector<_Char_> mainArr(sizeForBenchmarkMain);
        std::vector<_Char_> subArr(sizeForBenchmarkSub);

        std::fill(subArr.begin(), subArr.end(), static_cast<_Char_>(2));
        std::fill(mainArr.begin(), mainArr.end(), static_cast<_Char_>(1));

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain) {
            const size_t pos = sizeForBenchmarkMain / 3;

            for (size_t i = 0; i < sizeForBenchmarkSub / 2; ++i)
                mainArr[pos + i] = subArr[i];

            for (size_t i = sizeForBenchmarkSub / 2; i < sizeForBenchmarkSub; ++i)
                mainArr[pos + i] = static_cast<_Char_>(3);
        }

        while (state.KeepRunning()) {
            auto* result = raze::algorithm::find_end(
                mainArr.data(), mainArr.data() + sizeForBenchmarkMain,
                subArr.data(), subArr.data() + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void SubWithManyFalsePositives(benchmark::State& state) noexcept {
        std::vector<_Char_> mainArr(sizeForBenchmarkMain);
        std::vector<_Char_> subArr(sizeForBenchmarkSub);

        for (size_t i = 0; i < sizeForBenchmarkSub; ++i)
            subArr[i] = static_cast<_Char_>(5 + i * 2);

        for (size_t i = 0; i < sizeForBenchmarkMain; ++i) {
            if (i % 3 == 0)
                mainArr[i] = subArr[0];
            else if (i % 5 == 0)
                mainArr[i] = subArr[sizeForBenchmarkSub - 1];
            else
                mainArr[i] = static_cast<_Char_>(100 + (i % 50));
        }

        while (state.KeepRunning()) {
            auto* result = raze::algorithm::find_end(
                mainArr.data(), mainArr.data() + sizeForBenchmarkMain,
                subArr.data(), subArr.data() + sizeForBenchmarkSub);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }

    static void RandomData(benchmark::State& state) noexcept {
        std::vector<_Char_> mainArr(sizeForBenchmarkMain);
        std::vector<_Char_> subArr(sizeForBenchmarkSub);

        fill_random(mainArr);
        fill_random(subArr);

        while (state.KeepRunning()) {
            auto* result = raze::algorithm::find_end(
                mainArr.data(), mainArr.data() + sizeForBenchmarkMain,
                subArr.data(), subArr.data() + sizeForBenchmarkSub);
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

REGISTER_ALL(SubWithPartialRepeats)
REGISTER_ALL(SubWithManyFalsePositives)
REGISTER_ALL(RandomData)

RAZE_BENCHMARK_MAIN();
