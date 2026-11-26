#include <raze/algorithm/fill/FillN.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_fill_n(benchmark::State& state, Fn&& fn) {
    auto data = rb::make_iota<T>(rb::arg_size(state));
    const auto n = static_cast<std::ptrdiff_t>(data.size());
    T val = rb::needle_value<T>();
    T next_val = rb::alt_value<T>();

    rb::bench_inplace<T>(state, data, [&](auto& d) {
        auto res = fn(d, n, val);
        std::swap(val, next_val);
        return res;
    });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("FillN",
        [](benchmark::State& s) { run_fill_n<T>(s, [](auto& d, std::ptrdiff_t n, T v) { return raze::algorithm::fill_n(d.begin(), n, v); }); },
        [](benchmark::State& s) { run_fill_n<T>(s, [](auto& d, std::ptrdiff_t n, T v) { return std::ranges::fill_n(d.begin(), n, v); }); },
        [](benchmark::State& s) { run_fill_n<T>(s, [](auto& d, std::ptrdiff_t n, T v) { return raze::algorithm::fill_n[raze::options::fscalar](d.begin(), n, v); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

