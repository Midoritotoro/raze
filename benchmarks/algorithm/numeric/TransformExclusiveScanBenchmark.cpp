#include <raze/algorithm/numeric/TransformExclusiveScan.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_transform_exclusive_scan(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_bounded_sum_data<T>(rb::arg_size(state));
    rb::bench_copy<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    auto uop = [](T x) { return static_cast<T>(x * T(2)); };

    rb::register_triplet<T>("TransformExclusiveScan",
        [uop](benchmark::State& s) { run_transform_exclusive_scan<T>(s, [uop](const auto& in, auto& out) { return raze::algorithm::transform_exclusive_scan(in, out.begin(), T(0), std::plus<>{}, uop); }); },
        [uop](benchmark::State& s) { run_transform_exclusive_scan<T>(s, [uop](const auto& in, auto& out) { return std::transform_exclusive_scan(in.begin(), in.end(), out.begin(), T(0), std::plus<>{}, uop); }); },
        [uop](benchmark::State& s) { run_transform_exclusive_scan<T>(s, [uop](const auto& in, auto& out) { return raze::algorithm::transform_exclusive_scan[raze::options::fscalar](in, out.begin(), T(0), std::plus<>{}, uop); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

