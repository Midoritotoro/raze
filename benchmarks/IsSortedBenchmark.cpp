#include <raze/algorithm/order/IsSorted.h>
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
    std::vector<T> make_sorted_data(std::size_t size, bool descending = false) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i) {
            const auto v = (i * 120) / size;  
            data[i] = static_cast<T>(descending ? (120 - v) : v);
        }
        return data;
    }
}

template <class T>
static void BM_StdIsSorted(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_sorted_data<T>(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = std::ranges::is_sorted(data);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeIsSorted(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_sorted_data<T>(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::is_sorted(data);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeIsSortedScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_sorted_data<T>(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::is_sorted[raze::options::fscalar](data);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_StdIsSortedPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_sorted_data<T>(size, true);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = std::ranges::is_sorted(data, std::greater<>{});
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeIsSortedPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_sorted_data<T>(size, true);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::is_sorted(data, std::greater<>{});
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeIsSortedPredScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto data = make_sorted_data<T>(size, true);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::is_sorted[raze::options::fscalar](data, std::greater<>{});
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

    reg("BM_RazeIsSorted",           &BM_RazeIsSorted<T>);
    reg("BM_StdIsSorted",            &BM_StdIsSorted<T>);
    reg("BM_RazeIsSortedScalar",     &BM_RazeIsSortedScalar<T>);

    reg("BM_RazeIsSortedPred",       &BM_RazeIsSortedPred<T>);
    reg("BM_StdIsSortedPred",        &BM_StdIsSortedPred<T>);
    reg("BM_RazeIsSortedPredScalar", &BM_RazeIsSortedPredScalar<T>);
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