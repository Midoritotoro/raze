#include <benchmark/benchmark.h>
#include <immintrin.h>
#include <benchmarks/tools/BenchmarkHelper.h>
#include <bit>
#include <limits>
#include <cstddef>

int* find_simd(int* b, int* e, int v) {
    size_t count = e - b;
    size_t aligned_size = count & ~size_t(15);
    __m512i comp = _mm512_set1_epi32(v);

    for (size_t i = 0; i != aligned_size; i += 16) {
        __m512i data = _mm512_loadu_si512(b + i);
        unsigned int mask = _mm512_cmpeq_epi32_mask(comp, data);

        if (mask != 0) return b + i + _tzcnt_u32(mask);
    }

    for (size_t i = aligned_size; i < count; ++i)
        if (b[i] == v)
            return b + i;

    return e;
}

int* find_scalar(int* b, int* e, int v) {
    for (; b != e; ++b)
        if (*b == v)
            break;

    return b;
}

template <std::size_t Size, std::size_t Position>
static void BM_FindSimd(benchmark::State& state) {
    TestData<int, Size> test;

    constexpr int needle = std::numeric_limits<int>::max();
    test.data[Position] = needle;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto it = find_simd(&test.data[0], &test.data[0] + Size, needle);
        benchmark::DoNotOptimize(it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(int));
}

template <std::size_t Size, std::size_t Position>
static void BM_FindScalar(benchmark::State& state) {
    TestData<int, Size> test;

    constexpr int needle = std::numeric_limits<int>::max();
    test.data[Position] = needle;

    for (auto _ : state) {
        benchmark::DoNotOptimize(test.data);
        auto it = find_scalar(&test.data[0], &test.data[0] + Size, needle);
        benchmark::DoNotOptimize(it);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * Size * sizeof(int));
}

#define BENCHMARK_FIND(name1, name2) \
    BENCHMARK(name1<16, 15>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<16, 15>)->Repetitions(10)->ReportAggregatesOnly(true); \
    \
    BENCHMARK(name1<1024, 1023>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<1024, 1023>)->Repetitions(10)->ReportAggregatesOnly(true); \
    \
    BENCHMARK(name1<4096, 4095>)->Repetitions(10)->ReportAggregatesOnly(true); \
    BENCHMARK(name2<4096, 4095>)->Repetitions(10)->ReportAggregatesOnly(true);

void RegisterAll() {
    BENCHMARK_FIND(BM_FindSimd, BM_FindScalar);
}

RAZE_BENCHMARK_MAIN();