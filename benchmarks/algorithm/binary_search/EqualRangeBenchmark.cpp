#include <raze/algorithm/binary_search/EqualRange.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_equal_range(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_sorted<T>(rb::arg_size(state));
    const T target = static_cast<T>(90);
    rb::bench_read<T>(state, data, [&](const auto& d) { return fn(d, target); });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("EqualRange",
        [](benchmark::State& s) { run_equal_range<T>(s, [](const auto& d, T v) { return raze::algorithm::equal_range(d, v); }); },
        [](benchmark::State& s) { run_equal_range<T>(s, [](const auto& d, T v) { return std::ranges::equal_range(d, v); }); },
        [](benchmark::State& s) { run_equal_range<T>(s, [](const auto& d, T v) { return raze::algorithm::equal_range[raze::options::fscalar](d, v); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

