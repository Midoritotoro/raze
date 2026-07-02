#include <raze/algorithm/copy/Copy.h>
#include <raze/algorithm/copy/CopyN.h>
#include <raze/algorithm/copy/CopyBackward.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <cstring>

struct S1 { char a; };

struct S2 { short a; };

struct S4 { int a; };

struct S4_shorts { short a; short b; };

struct S4_chars { char a; char b; char c; char d; };

struct S8 { int x; int y; };

struct S8_mixed { int a; float b; };

struct S8_floats { float a; float b; };

struct S8_double { double a; };

struct S8_ll { long long a; };

template <class T, std::size_t Size>
static void BM_StdCopy(benchmark::State& state) {
    TestData<T, Size> src;
    std::array<T, Size> dst{};

    for (auto _ : state) {
        benchmark::DoNotOptimize(src.data);
        benchmark::DoNotOptimize(dst);

        std::ranges::copy(src.data, dst.begin());

        benchmark::DoNotOptimize(dst);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_RazeCopy(benchmark::State& state) {
    TestData<T, Size> src;
    std::array<T, Size> dst{};

    for (auto _ : state) {
        benchmark::DoNotOptimize(src.data);
        benchmark::DoNotOptimize(dst);

        raze::algorithm::copy(src.data, dst.begin());

        benchmark::DoNotOptimize(dst);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_StdCopyStruct(benchmark::State& state) {
    std::array<T, Size> src{};
    std::array<T, Size> dst{};
    for (auto& x : src) std::memset(&x, 0xAB, sizeof(T));

    for (auto _ : state) {
        benchmark::DoNotOptimize(src);
        benchmark::DoNotOptimize(dst);

        std::ranges::copy(src, dst.begin());

        benchmark::DoNotOptimize(dst);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

template <class T, std::size_t Size>
static void BM_RazeCopyStruct(benchmark::State& state) {
    std::array<T, Size> src{};
    std::array<T, Size> dst{};
    for (auto& x : src) std::memset(&x, 0xAB, sizeof(T));

    for (auto _ : state) {
        benchmark::DoNotOptimize(src);
        benchmark::DoNotOptimize(dst);

        raze::algorithm::copy(src, dst.begin());

        benchmark::DoNotOptimize(dst);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(T) * 2);
}

#define RAZE_BENCHMARK_COPY(name1, name2) \
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

#define RAZE_BENCHMARK_COPY_STRUCT_1_2(name1, name2) \
    BENCHMARK(name1<S1, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S1, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S2, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S2, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S1, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S1, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S2, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S2, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S1, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S1, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S2, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S2, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S1, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S1, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S2, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S2, 16384>)->Repetitions(10)->ReportAggregatesOnly(true)

#define RAZE_BENCHMARK_COPY_STRUCT_4(name1, name2) \
    BENCHMARK(name1<S4, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S4, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S4_shorts, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S4_shorts, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S4_chars, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S4_chars, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S4, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S4, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S4_shorts, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S4_shorts, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S4_chars, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S4_chars, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S4, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S4, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S4_shorts, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S4_shorts, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S4_chars, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S4_chars, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S4, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S4, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S4_shorts, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S4_shorts, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S4_chars, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S4_chars, 16384>)->Repetitions(10)->ReportAggregatesOnly(true)

#define RAZE_BENCHMARK_COPY_STRUCT_8(name1, name2) \
    BENCHMARK(name1<S8, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8_mixed, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8_mixed, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8_floats, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8_floats, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8_double, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8_double, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8_ll, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8_ll, 16>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<S8, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8_mixed, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8_mixed, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8_floats, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8_floats, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8_double, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8_double, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8_ll, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8_ll, 1024>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<S8, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8_mixed, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8_mixed, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8_floats, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8_floats, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8_double, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8_double, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8_ll, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8_ll, 4096>)->Repetitions(10)->ReportAggregatesOnly(true);\
        \
    BENCHMARK(name1<S8, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8_mixed, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8_mixed, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8_floats, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8_floats, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8_double, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8_double, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name1<S8_ll, 16384>)->Repetitions(10)->ReportAggregatesOnly(true);\
    BENCHMARK(name2<S8_ll, 16384>)->Repetitions(10)->ReportAggregatesOnly(true)

void RegisterAll()
{
    RAZE_BENCHMARK_COPY(BM_RazeCopy, BM_StdCopy);
    RAZE_BENCHMARK_COPY_STRUCT_1_2(BM_RazeCopyStruct, BM_StdCopyStruct);
    RAZE_BENCHMARK_COPY_STRUCT_4(BM_RazeCopyStruct, BM_StdCopyStruct);
    RAZE_BENCHMARK_COPY_STRUCT_8(BM_RazeCopyStruct, BM_StdCopyStruct);
}

RAZE_BENCHMARK_MAIN();