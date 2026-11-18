#include <raze/algorithm/replace/Replace.h>
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
    std::vector<T> make_replace_data(std::size_t size, T val_a) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i)
            data[i] = (i & 1) ? val_a : static_cast<T>((i % 100) + 1);
        return data;
    }

    template <class T>
    std::vector<T> make_replace_if_data(std::size_t size) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i)
            data[i] = (i & 1) ? T(-1) : T(1);
        return data;
    }
}

template <class T>
static void BM_StdReplace(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    T old_v = std::numeric_limits<T>::max();
    T new_v = static_cast<T>(std::numeric_limits<T>::max() - 1);
    auto data = make_replace_data<T>(size, old_v);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        std::ranges::replace(data, old_v, new_v);
        std::swap(old_v, new_v);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeReplace(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    T old_v = std::numeric_limits<T>::max();
    T new_v = static_cast<T>(std::numeric_limits<T>::max() - 1);
    auto data = make_replace_data<T>(size, old_v);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        raze::algorithm::replace(data, old_v, new_v);
        std::swap(old_v, new_v);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeReplaceScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    T old_v = std::numeric_limits<T>::max();
    T new_v = static_cast<T>(std::numeric_limits<T>::max() - 1);
    auto data = make_replace_data<T>(size, old_v);

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        raze::algorithm::replace[raze::options::fscalar](data, old_v, new_v);
        std::swap(old_v, new_v);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_StdReplacePred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    auto data = make_replace_if_data<T>(size);
    auto pred = [](auto x) { return x > T(0); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        std::ranges::replace_if(data, pred, T(2));
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeReplacePred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    auto data = make_replace_if_data<T>(size);
    auto pred = [](auto x) { return x > T(0); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        raze::algorithm::replace_if(data, pred, T(2));
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeReplacePredScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    auto data = make_replace_if_data<T>(size);
    auto pred = [](auto x) { return x > T(0); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(data);
        raze::algorithm::replace_if[raze::options::fscalar](data, pred, T(2));
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

    reg("BM_RazeReplace",           &BM_RazeReplace<T>);
    reg("BM_StdReplace",            &BM_StdReplace<T>);
    reg("BM_RazeReplaceScalar",     &BM_RazeReplaceScalar<T>);

    reg("BM_RazeReplacePred",       &BM_RazeReplacePred<T>);
    reg("BM_StdReplacePred",        &BM_StdReplacePred<T>);
    reg("BM_RazeReplacePredScalar", &BM_RazeReplacePredScalar<T>);
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