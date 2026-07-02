#include <raze/algorithm/fill/Fill.h>
#include <raze/algorithm/fill/FillN.h>
#include <benchmarks/tools/BenchmarkHelper.h>

struct S1 { char a; };
struct S2 { short a; };
struct S4 { int a; };
struct S8 { int x, y; };

template <class T, std::size_t Size>
static void BM_StdFill(benchmark::State& state) {
    TestData<T, Size> test;
    constexpr T val = T(42);

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        std::ranges::fill(test.data, val);
        benchmark::DoNotOptimize(test.data);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_RazeFill(benchmark::State& state) {
    TestData<T, Size> test;
    constexpr T val = T(42);

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        raze::algorithm::fill(test.data, val);
        benchmark::DoNotOptimize(test.data);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_StdFillN(benchmark::State& state) {
    TestData<T, Size> test;
    constexpr T val = T(42);

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        std::fill_n(std::ranges::begin(test.data), Size, val);
        benchmark::DoNotOptimize(test.data);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_RazeFillN(benchmark::State& state) {
    TestData<T, Size> test;
    constexpr T val = T(42);

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        raze::algorithm::fill_n(std::ranges::begin(test.data), Size, val);
        benchmark::DoNotOptimize(test.data);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_StdFillStruct(benchmark::State& state) {
    std::array<T, Size> data{};
    T val{};
    std::memset(&val, 0xAB, sizeof(T));

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        std::ranges::fill(data, val);
        benchmark::DoNotOptimize(data);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_RazeFillStruct(benchmark::State& state) {
    std::array<T, Size> data{};
    T val{};
    std::memset(&val, 0xAB, sizeof(T));

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        raze::algorithm::fill(data, val);
        benchmark::DoNotOptimize(data);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_StdFillNStruct(benchmark::State& state) {
    std::array<T, Size> data{};
    T val{};
    std::memset(&val, 0xAB, sizeof(T));

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        std::fill_n(data.begin(), Size, val);
        benchmark::DoNotOptimize(data);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

template <class T, std::size_t Size>
static void BM_RazeFillNStruct(benchmark::State& state) {
    std::array<T, Size> data{};
    T val{};
    std::memset(&val, 0xAB, sizeof(T));

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        raze::algorithm::fill_n(data.begin(), Size, val);
        benchmark::DoNotOptimize(data);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T));
}

#define RAZE_BENCHMARK_FILL(name1, name2) \
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

#define RAZE_BENCHMARK_FILL_STRUCT(name1, name2) \
    BENCHMARK(name1<S1, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S1, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S2, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S2, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S4, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S4, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<S1, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S1, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S2, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S2, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S4, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S4, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<S1, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S1, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S2, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S2, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S4, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S4, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<S1, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S1, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S2, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S2, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S4, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S4, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8, 16384>)->Repetitions(10)->ReportAggregatesOnly(true)

void RegisterAll()
{
    RAZE_BENCHMARK_FILL_STRUCT(BM_RazeFillStruct, BM_StdFillStruct);
    RAZE_BENCHMARK_FILL_STRUCT(BM_RazeFillNStruct, BM_StdFillNStruct);

    RAZE_BENCHMARK_FILL(BM_RazeFill, BM_StdFill);
    RAZE_BENCHMARK_FILL(BM_RazeFillN, BM_StdFillN);
}

RAZE_BENCHMARK_MAIN();