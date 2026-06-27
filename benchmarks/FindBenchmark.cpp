#include <raze/algorithm/find/Find.h>
#include <benchmarks/tools/BenchmarkHelper.h>

template <class T, std::size_t Size, std::size_t Position>
static void BM_StdFind(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T needle = std::numeric_limits<T>::max();
    test.data[Position] = needle;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto it = std::ranges::find(test.data, needle);
        benchmark::DoNotOptimize(it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}


template <class T, std::size_t Size, std::size_t Position>
static void BM_RazeFind(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T needle = std::numeric_limits<T>::max();

    test.data[Position] = needle;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto it = raze::algorithm::find(test.data, needle);
        benchmark::DoNotOptimize(it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size, std::size_t Position>
static void BM_StdFindPred(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T needle = std::numeric_limits<T>::max();
    test.data[Position] = needle;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto it = std::ranges::find_if(test.data, [](auto x) { return ((x * 3 + 7) * 2) == 2; });
        benchmark::DoNotOptimize(it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}


template <class T, std::size_t Size, std::size_t Position>
static void BM_RazeFindPred(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T needle = std::numeric_limits<T>::max();

    test.data[Position] = needle;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto it = raze::algorithm::find_if(test.data, [](auto x) { return ((x * 3 + 7) * 2) == 2; });
        benchmark::DoNotOptimize(it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

#define RAZE_BENCHMARK_FIND(name1, name2) \
    BENCHMARK(name1<raze::i8, 16, 15>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 16, 15>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 16, 15>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 16, 15>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 16, 15>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 16, 15>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 16, 15>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 16, 15>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 16, 15>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 16, 15>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 16, 15>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 16, 15>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 1024, 1023>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 1024, 1023>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 1024, 1023>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 1024, 1023>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 1024, 1023>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 1024, 1023>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 1024, 1023>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 1024, 1023>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 1024, 1023>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 1024, 1023>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 1024, 1023>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 1024, 1023>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 4096, 4095>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 4096, 4095>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 4096, 4095>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 4096, 4095>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 4096, 4095>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 4096, 4095>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 4096, 4095>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 4096, 4095>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 4096, 4095>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 4096, 4095>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 4096, 4095>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 4096, 4095>)->Repetitions(10)->ReportAggregatesOnly(true);

void RegisterAll()
{
    RAZE_BENCHMARK_FIND(BM_RazeFind, BM_StdFind);
    RAZE_BENCHMARK_FIND(BM_RazeFindPred, BM_StdFindPred);
}


RAZE_BENCHMARK_MAIN();