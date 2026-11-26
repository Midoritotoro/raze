#include <raze/algorithm/uninitialized/UninitializedFill.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_uninit_fill(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    T val = rb::needle_value<T>();
    T next = rb::alt_value<T>();
    rb::bench_raw_storage<T>(state, size, [&](T* raw, raze::sizetype n) {
        auto res = fn(raw, raw + n, val);
        benchmark::DoNotOptimize(res);
        std::swap(val, next);
    });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("UninitializedFill",
        [](benchmark::State& s) { run_uninit_fill<T>(s, [](T* f, T* l, T v) { return raze::algorithm::uninitialized_fill(f, l, v); }); },
        [](benchmark::State& s) { run_uninit_fill<T>(s, [](T* f, T* l, T v) { return std::ranges::uninitialized_fill(f, l, v); }); },
        [](benchmark::State& s) { run_uninit_fill<T>(s, [](T* f, T* l, T v) { return raze::algorithm::uninitialized_fill[raze::options::fscalar](f, l, v); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

