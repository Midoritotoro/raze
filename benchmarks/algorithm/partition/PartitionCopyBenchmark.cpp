#include <raze/algorithm/partition/PartitionCopy.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_partition_copy(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_half_match<T>(rb::arg_size(state));
    rb::bench_partition_copy<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("PartitionCopy",
        [](benchmark::State& s) { run_partition_copy<T>(s, [](const auto& in, auto& ot, auto& of) { return raze::algorithm::partition_copy(in, ot.begin(), of.begin(), rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_partition_copy<T>(s, [](const auto& in, auto& ot, auto& of) { return std::ranges::partition_copy(in, ot.begin(), of.begin(), rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_partition_copy<T>(s, [](const auto& in, auto& ot, auto& of) { return raze::algorithm::partition_copy[raze::options::fscalar](in, ot.begin(), of.begin(), rb::unary_pred<T>()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

