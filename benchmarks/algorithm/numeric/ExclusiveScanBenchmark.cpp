#include <raze/algorithm/numeric/ExclusiveScan.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_exclusive_scan(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_bounded_sum_data<T>(rb::arg_size(state));
    rb::bench_copy<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("ExclusiveScan",
        [](benchmark::State& s) { run_exclusive_scan<T>(s, [](const auto& in, auto& out) { return raze::algorithm::exclusive_scan(in, out.begin(), T(0)); }); },
        [](benchmark::State& s) { run_exclusive_scan<T>(s, [](const auto& in, auto& out) { return std::exclusive_scan(in.begin(), in.end(), out.begin(), T(0)); }); },
        [](benchmark::State& s) { run_exclusive_scan<T>(s, [](const auto& in, auto& out) { return raze::algorithm::exclusive_scan[raze::options::fscalar](in, out.begin(), T(0)); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

