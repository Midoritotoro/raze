#include <raze/algorithm/copy/CopyIf.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_copy_if(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_half_match<T>(rb::arg_size(state));
    rb::bench_copy<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("CopyIf",
        [](benchmark::State& s) { run_copy_if<T>(s, [](const auto& in, auto& out) { return raze::algorithm::copy_if(in, out.begin(), rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_copy_if<T>(s, [](const auto& in, auto& out) { return std::ranges::copy_if(in, out.begin(), rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_copy_if<T>(s, [](const auto& in, auto& out) { return raze::algorithm::copy_if[raze::options::fscalar](in, out.begin(), rb::unary_pred<T>()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();