#include <raze/algorithm/find/Equal.h>
#include <benchmarks/tools/BenchmarkHelper.h>

template <class T, std::size_t Size>
struct EqualTestData {
    std::array<T, Size> data1;
    std::array<T, Size> data2;

    EqualTestData() {
        for (std::size_t i = 0; i < Size; ++i) {
            data1[i] = static_cast<T>(i + 1);
            data2[i] = static_cast<T>(i + 1);
        }
    }
};

template <class T, std::size_t Size>
static void BM_StdEqual(benchmark::State& state) {
    EqualTestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data1);
        benchmark::DoNotOptimize(test.data2);

        auto result = std::ranges::equal(test.data1, test.data2);
        benchmark::DoNotOptimize(result);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_RazeEqual(benchmark::State& state) {
    EqualTestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data1);
        benchmark::DoNotOptimize(test.data2);

        auto result = raze::algorithm::equal(test.data1, test.data2);
        benchmark::DoNotOptimize(result);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_StdEqualPred(benchmark::State& state) {
    EqualTestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data1);
        benchmark::DoNotOptimize(test.data2);

        auto result = std::ranges::equal(test.data1, test.data2, [](auto a, auto b) { return (a * 2 + 1) == (b * 2 + 1); });
        benchmark::DoNotOptimize(result);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_RazeEqualPred(benchmark::State& state) {
    EqualTestData<T, Size> test;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data1);
        benchmark::DoNotOptimize(test.data2);

        auto result = raze::algorithm::equal(test.data1, test.data2, [](auto a, auto b) { return (a * 2 + 1) == (b * 2 + 1); });
        benchmark::DoNotOptimize(result);

        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

#define RAZE_BENCHMARK_EQUAL(name1, name2) \
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
    BENCHMARK(name2<raze::f64, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);

void RegisterAll()
{
    RAZE_BENCHMARK_EQUAL(BM_RazeEqual, BM_StdEqual);
    RAZE_BENCHMARK_EQUAL(BM_RazeEqualPred, BM_StdEqualPred);
}

RAZE_BENCHMARK_MAIN();