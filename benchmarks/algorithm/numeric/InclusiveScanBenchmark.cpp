#include <raze/algorithm/numeric/InclusiveScan.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_inclusive_scan(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_bounded_sum_data<T>(rb::arg_size(state));
    rb::bench_copy<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("InclusiveScan",
        [](benchmark::State& s) { run_inclusive_scan<T>(s, [](const auto& in, auto& out) { return raze::algorithm::inclusive_scan(in, out.begin()); }); },
        [](benchmark::State& s) { run_inclusive_scan<T>(s, [](const auto& in, auto& out) { return std::inclusive_scan(in.begin(), in.end(), out.begin()); }); },
        [](benchmark::State& s) { run_inclusive_scan<T>(s, [](const auto& in, auto& out) { return raze::algorithm::inclusive_scan[raze::options::fscalar](in, out.begin()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

