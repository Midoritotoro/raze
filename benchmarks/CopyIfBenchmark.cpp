#include <raze/algorithm/copy/CopyIf.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace {
    template <class T>
    std::vector<T> make_copy_if_data(std::size_t size) {
        std::vector<T> data(size);
        for (std::size_t i = 0; i < size; ++i)
            data[i] = (i & 1) ? T(1) : T(-1);  
        return data;
    }
}

template <class T>
static void BM_StdCopyIf(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto in = make_copy_if_data<T>(size);
    std::vector<T> out(size);
    auto pred = [](auto x) { return x > T(0); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = std::ranges::copy_if(in, out.begin(), pred);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeCopyIf(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto in = make_copy_if_data<T>(size);
    std::vector<T> out(size);
    auto pred = [](auto x) { return x > T(0); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = raze::algorithm::copy_if(in, out.begin(), pred);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(size * sizeof(T)));
}

template <class T>
static void BM_RazeCopyIfScalar(benchmark::State& state) {
    const auto size = static_cast<std::size_t>(state.range(0));
    const auto in = make_copy_if_data<T>(size);
    std::vector<T> out(size);
    auto pred = [](auto x) { return x > T(0); };

    for (auto _ : state) {
        benchmark::DoNotOptimize(in);
        auto res = raze::algorithm::copy_if[raze::options::fscalar](in, out.begin(), pred);
        benchmark::DoNotOptimize(res);
        benchmark::DoNotOptimize(out);
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

    reg("BM_RazeCopyIf",       &BM_RazeCopyIf<T>);
    reg("BM_StdCopyIf",        &BM_StdCopyIf<T>);
    reg("BM_RazeCopyIfScalar", &BM_RazeCopyIfScalar<T>);
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