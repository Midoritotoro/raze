#include <raze/algorithm/binary_search/LowerBound.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_lower_bound(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_sorted<T>(rb::arg_size(state));
    const T target = static_cast<T>(90);
    rb::bench_read<T>(state, data, [&](const auto& d) { return fn(d, target); });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("LowerBound",
        [](benchmark::State& s) { run_lower_bound<T>(s, [](const auto& d, T v) { return raze::algorithm::lower_bound(d, v); }); },
        [](benchmark::State& s) { run_lower_bound<T>(s, [](const auto& d, T v) { return std::ranges::lower_bound(d, v); }); },
        [](benchmark::State& s) { run_lower_bound<T>(s, [](const auto& d, T v) { return raze::algorithm::lower_bound[raze::options::fscalar](d, v); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

