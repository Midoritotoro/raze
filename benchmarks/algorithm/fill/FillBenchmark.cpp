#include <raze/algorithm/fill/Fill.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_fill(benchmark::State& state, Fn&& fn) {
    auto data = rb::make_iota<T>(rb::arg_size(state));
    T val = rb::needle_value<T>();
    T next_val = rb::alt_value<T>();

    rb::bench_inplace<T>(state, data, [&](auto& d) {
        auto res = fn(d, val);
        std::swap(val, next_val);
        return res;
    });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Fill",
        [](benchmark::State& s) { run_fill<T>(s, [](auto& d, T v) { return raze::algorithm::fill(d, v); }); },
        [](benchmark::State& s) { run_fill<T>(s, [](auto& d, T v) { return std::ranges::fill(d, v); }); },
        [](benchmark::State& s) { run_fill<T>(s, [](auto& d, T v) { return raze::algorithm::fill[raze::options::fscalar](d, v); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

