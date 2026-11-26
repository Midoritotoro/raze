#include <raze/algorithm/extreme/Max.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_max(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_extreme_data<T>(rb::arg_size(state));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Max",
        [](benchmark::State& s) { run_max<T>(s, [](const auto& d) { return raze::algorithm::max(d); }); },
        [](benchmark::State& s) { run_max<T>(s, [](const auto& d) { return std::ranges::max(d); }); },
        [](benchmark::State& s) { run_max<T>(s, [](const auto& d) { return raze::algorithm::max[raze::options::fscalar](d); }); });

    rb::register_triplet<T>("MaxComp",
        [](benchmark::State& s) { run_max<T>(s, [](const auto& d) { return raze::algorithm::max(d, rb::custom_comp()); }); },
        [](benchmark::State& s) { run_max<T>(s, [](const auto& d) { return std::ranges::max(d, rb::custom_comp()); }); },
        [](benchmark::State& s) { run_max<T>(s, [](const auto& d) { return raze::algorithm::max[raze::options::fscalar](d, rb::custom_comp()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

