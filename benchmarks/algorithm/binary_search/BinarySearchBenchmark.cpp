#include <raze/algorithm/binary_search/BinarySearch.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_binary_search(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_sorted<T>(rb::arg_size(state));
    const T target = static_cast<T>(90);
    rb::bench_read<T>(state, data, [&](const auto& d) { return fn(d, target); });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("BinarySearch",
        [](benchmark::State& s) { run_binary_search<T>(s, [](const auto& d, T v) { return raze::algorithm::binary_search(d, v); }); },
        [](benchmark::State& s) { run_binary_search<T>(s, [](const auto& d, T v) { return std::ranges::binary_search(d, v); }); },
        [](benchmark::State& s) { run_binary_search<T>(s, [](const auto& d, T v) { return raze::algorithm::binary_search[raze::options::fscalar](d, v); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

