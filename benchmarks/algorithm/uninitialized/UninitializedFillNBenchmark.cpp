#include <raze/algorithm/uninitialized/UninitializedFillN.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_uninit_fill_n(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    T val = rb::needle_value<T>();
    T next = rb::alt_value<T>();
    rb::bench_raw_storage<T>(state, size, [&](T* raw, raze::sizetype n) {
        auto res = fn(raw, static_cast<std::ptrdiff_t>(n), val);
        benchmark::DoNotOptimize(res);
        std::swap(val, next);
    });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("UninitializedFillN",
        [](benchmark::State& s) { run_uninit_fill_n<T>(s, [](T* f, std::ptrdiff_t n, T v) { return raze::algorithm::uninitialized_fill_n(f, n, v); }); },
        [](benchmark::State& s) { run_uninit_fill_n<T>(s, [](T* f, std::ptrdiff_t n, T v) { return std::ranges::uninitialized_fill_n(f, n, v); }); },
        [](benchmark::State& s) { run_uninit_fill_n<T>(s, [](T* f, std::ptrdiff_t n, T v) { return raze::algorithm::uninitialized_fill_n[raze::options::fscalar](f, n, v); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

