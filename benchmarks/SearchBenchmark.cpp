#include <raze/algorithm/find/Search.h>
#include <benchmarks/tools/BenchmarkHelper.h>

template <class T, std::size_t Size, std::size_t NeedleSize, std::size_t Position>
static void BM_StdSearch(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T needle_value = std::numeric_limits<T>::max();

    std::array<T, NeedleSize> needle {};
    needle.fill(needle_value);

    for (std::size_t i = 0; i < needle.size() && Position + i < Size; ++i)
        test.data[Position + i] = needle[i];

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);

        for (int i = 0; i < 1024; ++i) {
            auto it = std::ranges::search(test.data, needle);
            benchmark::DoNotOptimize(it);
        }

        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * Size);
}

template <class T, std::size_t Size, std::size_t NeedleSize, std::size_t Position>
static void BM_RazeSearch(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T needle_value = std::numeric_limits<T>::max();

    std::array<T, NeedleSize> needle {};
    needle.fill(needle_value);

    for (std::size_t i = 0; i < needle.size() && Position + i < Size; ++i)
        test.data[Position + i] = needle[i];

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);

        for (int i = 0; i < 1024; ++i) {
            auto it = raze::algorithm::search(test.data, needle);
            benchmark::DoNotOptimize(it);
        }

        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * Size);
}

template <class T, std::size_t Size, std::size_t NeedleSize, std::size_t Position>
static void BM_StdSearchPred(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T needle_value = std::numeric_limits<T>::max();

    std::array<T, NeedleSize> needle {};
    needle.fill(needle_value);

    for (std::size_t i = 0; i < needle.size() && Position + i < Size; ++i)
        test.data[Position + i] = needle[i];

    auto pred = [](auto x, auto y) {
        auto transform = [](auto v) {
            return (v * 1664525 + 1013904223) - 17;
        };

        return transform(x) == transform(y);
    };

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);

        for (int i = 0; i < 1024; ++i) {
            auto it = std::ranges::search(test.data, needle, pred);
            benchmark::DoNotOptimize(it);
        }

        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * Size);
}

template <class T, std::size_t Size, std::size_t NeedleSize, std::size_t Position>
static void BM_RazeSearchPred(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T needle_value = std::numeric_limits<T>::max();

    std::array<T, NeedleSize> needle {};
    needle.fill(needle_value);

    for (std::size_t i = 0; i < needle.size() && Position + i < Size; ++i)
        test.data[Position + i] = needle[i];

    auto pred = [](auto x, auto y) {
        auto transform = [](auto v) {
            return (v * 1664525 + 1013904223) - 17;
        };

        return transform(x) == transform(y);
    };

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);

        for (int i = 0; i < 1024; ++i) {
            auto it = raze::algorithm::search(test.data, needle, pred);
            benchmark::DoNotOptimize(it);
        }

        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * Size);
}

#define RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, size, needle, pos) \
    BENCHMARK(name1<raze::i8,  size, needle, pos>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i8,  size, needle, pos>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i16, size, needle, pos>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i16, size, needle, pos>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i32, size, needle, pos>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i32, size, needle, pos>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::i64, size, needle, pos>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::i64, size, needle, pos>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f32, size, needle, pos>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f32, size, needle, pos>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name1<raze::f64, size, needle, pos>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<raze::f64, size, needle, pos>)->Repetitions(10)->ReportAggregatesOnly(true);

#define RAZE_BENCHMARK_SEARCH(name1, name2) \
    /* Size = 16 */ \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 16, 1, 0) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 16, 1, 8) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 16, 1, 15) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 16, 4, 0) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 16, 4, 6) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 16, 4, 12) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 16, 8, 0) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 16, 8, 4) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 16, 8, 8) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 16, 16, 0) \
    \
    /* Size = 1024 */ \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 1024, 1, 0) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 1024, 1, 512) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 1024, 1, 1023) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 1024, 4, 0) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 1024, 4, 512) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 1024, 4, 1020) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 1024, 8, 0) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 1024, 8, 512) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 1024, 8, 1016) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 1024, 32, 0) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 1024, 32, 512) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 1024, 32, 992) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 1024, 1024, 0) \
    \
    /* Size = 4096 */ \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 4096, 1, 0) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 4096, 1, 2048) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 4096, 1, 4095) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 4096, 4, 0) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 4096, 4, 2048) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 4096, 4, 4092) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 4096, 8, 0) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 4096, 8, 2048) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 4096, 8, 4088) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 4096, 64, 0) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 4096, 64, 2048) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 4096, 64, 4032) \
    RAZE_BENCHMARK_SEARCH_TYPES(name1, name2, 4096, 4096, 0)

void RegisterAll()
{
    RAZE_BENCHMARK_SEARCH(BM_RazeSearch, BM_StdSearch);
    RAZE_BENCHMARK_SEARCH(BM_RazeSearchPred, BM_StdSearchPred);
}

RAZE_BENCHMARK_MAIN();