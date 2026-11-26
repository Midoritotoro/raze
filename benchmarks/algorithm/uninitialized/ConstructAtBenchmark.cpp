#include <raze/algorithm/uninitialized/ConstructAt.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_construct_at(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    T val = rb::needle_value<T>();
    T next = rb::alt_value<T>();
    rb::bench_raw_storage<T>(state, size, [&](T* raw, raze::sizetype n) {
        for (raze::sizetype i = 0; i < n; ++i)
            fn(raw + i, val);
        std::swap(val, next);
    });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("ConstructAt",
        [](benchmark::State& s) { run_construct_at<T>(s, [](T* p, T v) { return raze::algorithm::construct_at(p, v); }); },
        [](benchmark::State& s) { run_construct_at<T>(s, [](T* p, T v) { return std::ranges::construct_at(p, v); }); },
        [](benchmark::State& s) { run_construct_at<T>(s, [](T* p, T v) { return raze::algorithm::construct_at[raze::options::fscalar](p, v); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

