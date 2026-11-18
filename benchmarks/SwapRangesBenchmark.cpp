#include <raze/algorithm/swap/SwapRanges.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace {
    template <class T>
    std::vector<T> make_swap_data(std::size_t size, T offset) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i)
            data[i] = static_cast<T>((i % 100) + offset);
        return data;
    }
}

template <class T>
static void BM_StdSwapRanges(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    auto a = make_swap_data<T>(size, T(1));
    auto b = make_swap_data<T>(size, T(2));

    for (auto _ : state) {
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        auto res = std::ranges::swap_ranges(a, b);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void BM_RazeSwapRanges(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    auto a = make_swap_data<T>(size, T(1));
    auto b = make_swap_data<T>(size, T(2));

    for (auto _ : state) {
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        auto res = raze::algorithm::swap_ranges(a, b);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void BM_RazeSwapRangesScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    auto a = make_swap_data<T>(size, T(1));
    auto b = make_swap_data<T>(size, T(2));

    for (auto _ : state) {
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        auto res = raze::algorithm::swap_ranges[raze::options::fscalar](a, b);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void register_type_benchmarks(const char* type_name) {
    static constexpr std::array<int64_t, 6> sizes{ 16, 64, 256, 1024, 4096, 16384 };

    auto reg = [&](const char* fn_name, void(*fn)(benchmark::State&)) {
        const std::string full = std::string(fn_name) + "<" + type_name + ">";
        auto* bm = ::benchmark::RegisterBenchmark(full.c_str(), fn);
        for (int64_t s : sizes)
            bm->Args({ s });
        bm->MinTime(0.5)->Repetitions(10)->ReportAggregatesOnly(true);
    };

    reg("BM_RazeSwapRanges",       &BM_RazeSwapRanges<T>);
    reg("BM_StdSwapRanges",        &BM_StdSwapRanges<T>);
    reg("BM_RazeSwapRangesScalar", &BM_RazeSwapRangesScalar<T>);
}

void RegisterAll() {
    register_type_benchmarks<raze::i8>("raze::i8");
    register_type_benchmarks<raze::i16>("raze::i16");
    register_type_benchmarks<raze::i32>("raze::i32");
    register_type_benchmarks<raze::i64>("raze::i64");
    register_type_benchmarks<raze::f32>("raze::f32");
    register_type_benchmarks<raze::f64>("raze::f64");
}

RAZE_BENCHMARK_MAIN();