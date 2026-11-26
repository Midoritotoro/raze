#include <raze/algorithm/extreme/Min.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_min(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_extreme_data<T>(rb::arg_size(state));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Min",
        [](benchmark::State& s) { run_min<T>(s, [](const auto& d) { return raze::algorithm::min(d); }); },
        [](benchmark::State& s) { run_min<T>(s, [](const auto& d) { return std::ranges::min(d); }); },
        [](benchmark::State& s) { run_min<T>(s, [](const auto& d) { return raze::algorithm::min[raze::options::fscalar](d); }); });

    rb::register_triplet<T>("MinComp",
        [](benchmark::State& s) { run_min<T>(s, [](const auto& d) { return raze::algorithm::min(d, rb::custom_comp()); }); },
        [](benchmark::State& s) { run_min<T>(s, [](const auto& d) { return std::ranges::min(d, rb::custom_comp()); }); },
        [](benchmark::State& s) { run_min<T>(s, [](const auto& d) { return raze::algorithm::min[raze::options::fscalar](d, rb::custom_comp()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

