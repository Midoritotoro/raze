#include <raze/algorithm/find/FindLast.h>
#include <benchmarks/tools/BenchmarkHelper.h>

template <class T, std::size_t Size, std::size_t Position>
static void BM_StdFindLast(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T needle = std::numeric_limits<T>::max();
    test.data[Position] = needle;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);

        for (int i = 0; i < 1024; ++i) {
            auto it = std::ranges::find_last(test.data, needle);
            benchmark::DoNotOptimize(it);
        }

        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * Size);
}


template <class T, std::size_t Size, std::size_t Position>
static void BM_RazeFindLast(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T needle = std::numeric_limits<T>::max();

    test.data[Position] = needle;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);

        for (int i = 0; i < 1024; ++i) {
            auto it = raze::algorithm::find_last(test.data, needle);
            benchmark::DoNotOptimize(it);
        }
        
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * Size);
}

template <class T, std::size_t Size, std::size_t Position>
static void BM_StdFindLastPred(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T needle = std::numeric_limits<T>::max();
    test.data[Position] = needle;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);

        for (int i = 0; i < 1024; ++i) {
            auto it = std::ranges::find_last_if(test.data, [](auto x) { return ((x * 3 + 7) * 2) == 2; });
            benchmark::DoNotOptimize(it);
        }

        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * Size);
}


template <class T, std::size_t Size, std::size_t Position>
static void BM_RazeFindLastPred(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T needle = std::numeric_limits<T>::max();

    test.data[Position] = needle;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);

        for (int i = 0; i < 1024; ++i) {
            auto it = raze::algorithm::find_last_if(test.data, [](auto x) { return ((x * 3 + 7) * 2) == 2; });
            benchmark::DoNotOptimize(it);
        }

        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * Size);
}

#define RAZE_BENCHMARK_FIND(name1, name2) \
    BENCHMARK(name1<raze::i8, 16, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 16, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 16, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 16, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 16, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 16, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 16, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 16, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 16, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 16, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 16, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 16, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 1024, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 1024, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 1024, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 1024, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 1024, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 1024, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 1024, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 1024, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 1024, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 1024, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 1024, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 1024, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 4096, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 4096, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 4096, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 4096, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 4096, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 4096, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 4096, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 4096, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 4096, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 4096, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 4096, 0>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 4096, 0>)->Repetitions(10)->ReportAggregatesOnly(true);

void RegisterAll()
{
    RAZE_BENCHMARK_FIND(BM_RazeFindLast, BM_StdFindLast);
    RAZE_BENCHMARK_FIND(BM_RazeFindLastPred, BM_StdFindLastPred);
}


RAZE_BENCHMARK_MAIN();