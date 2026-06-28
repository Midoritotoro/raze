#include <raze/algorithm/replace/Replace.h>
#include <benchmarks/tools/BenchmarkHelper.h>

template <class T, std::size_t Size>
static void BM_StdReplace(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T old_val = T(0);
    constexpr T new_val = T(42);

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);

        std::ranges::replace(test.data, old_val, new_val);
        benchmark::DoNotOptimize(test.data);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_RazeReplace(benchmark::State& state) {
    TestData<T, Size> test;

    constexpr T old_val = T(0);
    constexpr T new_val = T(42);

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);

        raze::algorithm::replace(test.data, old_val, new_val);
        benchmark::DoNotOptimize(test.data);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_StdReplaceIf(benchmark::State& state) {
    TestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);

        std::ranges::replace_if(test.data, [](auto x) { return x > T(0); }, T(99));
        benchmark::DoNotOptimize(test.data);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_RazeReplaceIf(benchmark::State& state) {
    TestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);

        raze::algorithm::replace_if(test.data, [](auto x) { return x > T(0); }, T(99));
        benchmark::DoNotOptimize(test.data);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_StdReplaceIfComplex(benchmark::State& state) {
    TestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);

        std::ranges::replace_if(test.data, [](auto x) { return ((x * 3 + 7) - 5) == 0; }, T(0));
        benchmark::DoNotOptimize(test.data);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_RazeReplaceIfComplex(benchmark::State& state) {
    TestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);

        raze::algorithm::replace_if(test.data, [](auto x) { return ((x * 3 + 7) - 5) == 0; }, T(0));
        benchmark::DoNotOptimize(test.data);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

#define RAZE_BENCHMARK_REPLACE(name1, name2) \
    BENCHMARK(name1<raze::i8, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<raze::i8, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i8, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i16, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i16, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i32, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i32, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::i64, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::i64, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f32, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f32, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<raze::f64, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<raze::f64, 16384>)->Repetitions(10)->ReportAggregatesOnly(true)

void RegisterAll()
{
    RAZE_BENCHMARK_REPLACE(BM_RazeReplace, BM_StdReplace);
    RAZE_BENCHMARK_REPLACE(BM_RazeReplaceIf, BM_StdReplaceIf);
    RAZE_BENCHMARK_REPLACE(BM_RazeReplaceIfComplex, BM_StdReplaceIfComplex);
}

RAZE_BENCHMARK_MAIN();