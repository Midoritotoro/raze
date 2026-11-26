#include <raze/algorithm/sort/StableSort.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_stable_sort(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_unsorted<T>(rb::arg_size(state));
    rb::bench_inplace_reset<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("StableSort",
        [](benchmark::State& s) { run_stable_sort<T>(s, [](auto& d) { return raze::algorithm::stable_sort(d); }); },
        [](benchmark::State& s) { run_stable_sort<T>(s, [](auto& d) { return std::ranges::stable_sort(d); }); },
        [](benchmark::State& s) { run_stable_sort<T>(s, [](auto& d) { return raze::algorithm::stable_sort[raze::options::fscalar](d); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

