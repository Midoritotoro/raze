#include <raze/algorithm/remove/Unique.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_unique(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_unique_data<T>(rb::arg_size(state));
    rb::bench_inplace_reset<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Unique",
        [](benchmark::State& s) { run_unique<T>(s, [](auto& d) { return raze::algorithm::unique(d); }); },
        [](benchmark::State& s) { run_unique<T>(s, [](auto& d) { return std::ranges::unique(d); }); },
        [](benchmark::State& s) { run_unique<T>(s, [](auto& d) { return raze::algorithm::unique[raze::options::fscalar](d); }); });

    rb::register_triplet<T>("UniquePred",
        [](benchmark::State& s) { run_unique<T>(s, [](auto& d) { return raze::algorithm::unique(d, rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_unique<T>(s, [](auto& d) { return std::ranges::unique(d, rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_unique<T>(s, [](auto& d) { return raze::algorithm::unique[raze::options::fscalar](d, rb::binary_eq_pred<T>()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

