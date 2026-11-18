#include <raze/algorithm/order/IsSortedUntil.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace {
    template <class T>
    std::vector<T> make_sorted_until_data(std::size_t size, bool descending = false) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i) {
            const auto v = (i * 120) / size;
            data[i] = static_cast<T>(descending ? (120 - v) : v);
        }
        if (size > 0)
            data[size - 1] = descending ? T(125) : T(-1);  
        return data;
    }
}

template <class T>
static void BM_StdIsSortedUntil(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_sorted_until_data<T>(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = std::ranges::is_sorted_until(data);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeIsSortedUntil(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_sorted_until_data<T>(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::is_sorted_until(data);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeIsSortedUntilScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_sorted_until_data<T>(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::is_sorted_until[raze::options::fscalar](data);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_StdIsSortedUntilPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_sorted_until_data<T>(size, true);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = std::ranges::is_sorted_until(data, std::greater<>{});
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeIsSortedUntilPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_sorted_until_data<T>(size, true);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::is_sorted_until(data, std::greater<>{});
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeIsSortedUntilPredScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_sorted_until_data<T>(size, true);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::is_sorted_until[raze::options::fscalar](data, std::greater<>{});
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
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

    reg("BM_RazeIsSortedUntil",           &BM_RazeIsSortedUntil<T>);
    reg("BM_StdIsSortedUntil",            &BM_StdIsSortedUntil<T>);
    reg("BM_RazeIsSortedUntilScalar",     &BM_RazeIsSortedUntilScalar<T>);

    reg("BM_RazeIsSortedUntilPred",       &BM_RazeIsSortedUntilPred<T>);
    reg("BM_StdIsSortedUntilPred",        &BM_StdIsSortedUntilPred<T>);
    reg("BM_RazeIsSortedUntilPredScalar", &BM_RazeIsSortedUntilPredScalar<T>);
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