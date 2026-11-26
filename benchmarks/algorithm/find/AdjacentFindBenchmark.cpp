#include <raze/algorithm/find/AdjacentFind.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_adjacent_find(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_adjacent_data<T>(rb::arg_size(state));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("AdjacentFind",
        [](benchmark::State& s) { run_adjacent_find<T>(s, [](const auto& d) { return raze::algorithm::adjacent_find(d); }); },
        [](benchmark::State& s) { run_adjacent_find<T>(s, [](const auto& d) { return std::ranges::adjacent_find(d); }); },
        [](benchmark::State& s) { run_adjacent_find<T>(s, [](const auto& d) { return raze::algorithm::adjacent_find[raze::options::fscalar](d); }); });

    rb::register_triplet<T>("AdjacentFindPred",
        [](benchmark::State& s) { run_adjacent_find<T>(s, [](const auto& d) { return raze::algorithm::adjacent_find(d, rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_adjacent_find<T>(s, [](const auto& d) { return std::ranges::adjacent_find(d, rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_adjacent_find<T>(s, [](const auto& d) { return raze::algorithm::adjacent_find[raze::options::fscalar](d, rb::binary_eq_pred<T>()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();