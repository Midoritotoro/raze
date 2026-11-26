#include <raze/algorithm/sort/Sort.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_sort(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_unsorted<T>(rb::arg_size(state));
    rb::bench_inplace_reset<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Sort",
        [](benchmark::State& s) { run_sort<T>(s, [](auto& d) { return raze::algorithm::sort(d); }); },
        [](benchmark::State& s) { run_sort<T>(s, [](auto& d) { return std::ranges::sort(d); }); },
        [](benchmark::State& s) { run_sort<T>(s, [](auto& d) { return raze::algorithm::sort[raze::options::fscalar](d); }); });

    rb::register_triplet<T>("SortComp",
        [](benchmark::State& s) { run_sort<T>(s, [](auto& d) { return raze::algorithm::sort(d, rb::custom_comp()); }); },
        [](benchmark::State& s) { run_sort<T>(s, [](auto& d) { return std::ranges::sort(d, rb::custom_comp()); }); },
        [](benchmark::State& s) { run_sort<T>(s, [](auto& d) { return raze::algorithm::sort[raze::options::fscalar](d, rb::custom_comp()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

