#include <raze/algorithm/find/Mismatch.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace {
    template <class T>
    std::pair<std::vector<T>, std::vector<T>> make_mismatch_data(std::size_t size, std::size_t pos) {
        std::vector<T> a(size);
        for (std::size_t i = 0; i < size; ++i)
            a[i] = static_cast<T>(i % 100);

        auto b = a;
        if (pos < size)
            b[pos] = std::numeric_limits<T>::max();

        return { std::move(a), std::move(b) };
    }
}

template <class T>
static void BM_StdMismatch(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos  = static_cast<std::size_t>(state.range(1));
    auto [a, b] = make_mismatch_data<T>(size, pos);

    for (auto _ : state) {
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        auto res = std::ranges::mismatch(a, b);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void BM_RazeMismatch(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos  = static_cast<std::size_t>(state.range(1));
    auto [a, b] = make_mismatch_data<T>(size, pos);

    for (auto _ : state) {
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        auto res = raze::algorithm::mismatch(a, b);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void BM_RazeMismatchScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos  = static_cast<std::size_t>(state.range(1));
    auto [a, b] = make_mismatch_data<T>(size, pos);

    for (auto _ : state) {
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        auto res = raze::algorithm::mismatch[raze::options::fscalar](a, b);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void BM_StdMismatchPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos  = static_cast<std::size_t>(state.range(1));
    auto [a, b] = make_mismatch_data<T>(size, pos);
    auto pred = [](auto x, auto y) { return (x + T(1)) == (y + T(1)); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        auto res = std::ranges::mismatch(a, b, pred);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void BM_RazeMismatchPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos  = static_cast<std::size_t>(state.range(1));
    auto [a, b] = make_mismatch_data<T>(size, pos);
    auto pred = [](auto x, auto y) { return (x + T(1)) == (y + T(1)); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        auto res = raze::algorithm::mismatch(a, b, pred);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void BM_RazeMismatchPredScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos  = static_cast<std::size_t>(state.range(1));
    auto [a, b] = make_mismatch_data<T>(size, pos);
    auto pred = [](auto x, auto y) { return (x + T(1)) == (y + T(1)); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
        auto res = raze::algorithm::mismatch[raze::options::fscalar](a, b, pred);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void register_type_benchmarks(const char* type_name) {
    static constexpr std::array<std::array<int64_t, 2>, 6> cases{{
        { 16,    15    },
        { 64,    63    },
        { 256,   255   },
        { 1024,  1023  },
        { 4096,  4095  },
        { 16384, 16383 },
    }};

    auto reg = [&](const char* fn_name, void(*fn)(benchmark::State&)) {
        const std::string full = std::string(fn_name) + "<" + type_name + ">";
        auto* bm = ::benchmark::RegisterBenchmark(full.c_str(), fn);
        for (const auto& c : cases)
            bm->Args({ c[0], c[1] });
        bm->MinTime(0.5)->Repetitions(10)->ReportAggregatesOnly(true);
    };

    reg("BM_RazeMismatch",           &BM_RazeMismatch<T>);
    reg("BM_StdMismatch",            &BM_StdMismatch<T>);
    reg("BM_RazeMismatchScalar",     &BM_RazeMismatchScalar<T>);

    reg("BM_RazeMismatchPred",       &BM_RazeMismatchPred<T>);
    reg("BM_StdMismatchPred",        &BM_StdMismatchPred<T>);
    reg("BM_RazeMismatchPredScalar", &BM_RazeMismatchPredScalar<T>);
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