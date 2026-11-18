#include <raze/algorithm/find/FindLast.h>
#include <raze/algorithm/find/FindLastIf.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace {
    template <class T>
    std::vector<T> make_find_last_data(std::size_t size, std::size_t pos, T needle) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i)
            data[i] = static_cast<T>(i % 100);

        if (pos < size)
            data[pos] = needle;

        return data;
    }
}

template <class T>
static void BM_StdFindLast(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos  = static_cast<std::size_t>(state.range(1));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto data = make_find_last_data<T>(size, pos, needle);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = std::ranges::find_last(data, needle);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeFindLast(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos  = static_cast<std::size_t>(state.range(1));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto data = make_find_last_data<T>(size, pos, needle);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::find_last(data, needle);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeFindLastScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos  = static_cast<std::size_t>(state.range(1));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto data = make_find_last_data<T>(size, pos, needle);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::find_last[raze::options::fscalar](data, needle);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_StdFindLastPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos  = static_cast<std::size_t>(state.range(1));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto data = make_find_last_data<T>(size, pos, needle);
    auto pred = [](auto x) { return ((x * 3 + 7) * 2) == 2; };

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = std::ranges::find_last_if(data, pred);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeFindLastPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos  = static_cast<std::size_t>(state.range(1));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto data = make_find_last_data<T>(size, pos, needle);
    auto pred = [](auto x) { return ((x * 3 + 7) * 2) == 2; };

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::find_last_if(data, pred);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeFindLastPredScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto pos  = static_cast<std::size_t>(state.range(1));
    constexpr T needle = std::numeric_limits<T>::max();
    const auto data = make_find_last_data<T>(size, pos, needle);
    auto pred = [](auto x) { return ((x * 3 + 7) * 2) == 2; };

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        auto res = raze::algorithm::find_last_if[raze::options::fscalar](data, pred);
        benchmark::DoNotOptimize(res);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void register_type_benchmarks(const char* type_name) {
    static constexpr std::array<std::array<int64_t, 2>, 6> cases{{
        { 16,    0 },
        { 64,    0 },
        { 256,   0 },
        { 1024,  0 },
        { 4096,  0 },
        { 16384, 0 },
    }};

    auto reg = [&](const char* fn_name, void(*fn)(benchmark::State&)) {
        const std::string full = std::string(fn_name) + "<" + type_name + ">";
        auto* bm = ::benchmark::RegisterBenchmark(full.c_str(), fn);
        for (const auto& c : cases)
            bm->Args({ c[0], c[1] });
        bm->MinTime(0.5)->Repetitions(10)->ReportAggregatesOnly(true);
    };

    reg("BM_RazeFindLast",           &BM_RazeFindLast<T>);
    reg("BM_StdFindLast",            &BM_StdFindLast<T>);
    reg("BM_RazeFindLastScalar",     &BM_RazeFindLastScalar<T>);

    reg("BM_RazeFindLastPred",       &BM_RazeFindLastPred<T>);
    reg("BM_StdFindLastPred",        &BM_StdFindLastPred<T>);
    reg("BM_RazeFindLastPredScalar", &BM_RazeFindLastPredScalar<T>);
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