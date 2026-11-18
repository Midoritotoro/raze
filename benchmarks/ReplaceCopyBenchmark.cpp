#include <raze/algorithm/replace/ReplaceCopy.h>
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
    std::vector<T> make_replace_copy_data(std::size_t size, T target) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i)
            data[i] = (i & 1) ? target : static_cast<T>((i % 100) + 1);
        return data;
    }

    template <class T>
    std::vector<T> make_replace_copy_if_data(std::size_t size) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i)
            data[i] = (i & 1) ? T(-1) : T(1);
        return data;
    }
}

template <class T>
static void BM_StdReplaceCopy(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T old_v = std::numeric_limits<T>::max();
    constexpr T new_v = static_cast<T>(std::numeric_limits<T>::max() - 1);
    const auto in = make_replace_copy_data<T>(size, old_v);
    std::vector<T> out(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = std::ranges::replace_copy(in, out.begin(), old_v, new_v);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void BM_RazeReplaceCopy(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T old_v = std::numeric_limits<T>::max();
    constexpr T new_v = static_cast<T>(std::numeric_limits<T>::max() - 1);
    const auto in = make_replace_copy_data<T>(size, old_v);
    std::vector<T> out(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = raze::algorithm::replace_copy(in, out.begin(), old_v, new_v);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void BM_RazeReplaceCopyScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    constexpr T old_v = std::numeric_limits<T>::max();
    constexpr T new_v = static_cast<T>(std::numeric_limits<T>::max() - 1);
    const auto in = make_replace_copy_data<T>(size, old_v);
    std::vector<T> out(size);

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = raze::algorithm::replace_copy[raze::options::fscalar](in, out.begin(), old_v, new_v);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void BM_StdReplaceCopyPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto in = make_replace_copy_if_data<T>(size);
    std::vector<T> out(size);
    auto pred = [](auto x) { return x > T(0); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = std::ranges::replace_copy_if(in, out.begin(), pred, T(2));
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void BM_RazeReplaceCopyPred(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto in = make_replace_copy_if_data<T>(size);
    std::vector<T> out(size);
    auto pred = [](auto x) { return x > T(0); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = raze::algorithm::replace_copy_if(in, out.begin(), pred, T(2));
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T) * 2));
}

template <class T>
static void BM_RazeReplaceCopyPredScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto in = make_replace_copy_if_data<T>(size);
    std::vector<T> out(size);
    auto pred = [](auto x) { return x > T(0); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = raze::algorithm::replace_copy_if[raze::options::fscalar](in, out.begin(), pred, T(2));
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
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

    reg("BM_RazeReplaceCopy",           &BM_RazeReplaceCopy<T>);
    reg("BM_StdReplaceCopy",            &BM_StdReplaceCopy<T>);
    reg("BM_RazeReplaceCopyScalar",     &BM_RazeReplaceCopyScalar<T>);

    reg("BM_RazeReplaceCopyPred",       &BM_RazeReplaceCopyPred<T>);
    reg("BM_StdReplaceCopyPred",        &BM_StdReplaceCopyPred<T>);
    reg("BM_RazeReplaceCopyPredScalar", &BM_RazeReplaceCopyPredScalar<T>);
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