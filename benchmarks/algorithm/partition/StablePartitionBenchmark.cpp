#include <raze/algorithm/partition/StablePartition.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_stable_partition(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_half_match<T>(rb::arg_size(state));
    rb::bench_inplace_reset<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("StablePartition",
        [](benchmark::State& s) { run_stable_partition<T>(s, [](auto& d) { return raze::algorithm::stable_partition(d, rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_stable_partition<T>(s, [](auto& d) { return std::ranges::stable_partition(d, rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_stable_partition<T>(s, [](auto& d) { return raze::algorithm::stable_partition[raze::options::fscalar](d, rb::unary_pred<T>()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

