#include <raze/algorithm/remove/Remove.h>
#include <raze/algorithm/remove/RemoveIf.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_remove(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_half_match<T>(rb::arg_size(state));
    rb::bench_inplace_reset<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Remove",
        [](benchmark::State& s) { run_remove<T>(s, [](auto& d) { return raze::algorithm::remove(d, rb::needle_value<T>()); }); },
        [](benchmark::State& s) { run_remove<T>(s, [](auto& d) { return std::ranges::remove(d, rb::needle_value<T>()); }); },
        [](benchmark::State& s) { run_remove<T>(s, [](auto& d) { return raze::algorithm::remove[raze::options::fscalar](d, rb::needle_value<T>()); }); });

    rb::register_triplet<T>("RemovePred",
        [](benchmark::State& s) { run_remove<T>(s, [](auto& d) { return raze::algorithm::remove_if(d, rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_remove<T>(s, [](auto& d) { return std::ranges::remove_if(d, rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_remove<T>(s, [](auto& d) { return raze::algorithm::remove_if[raze::options::fscalar](d, rb::unary_pred<T>()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();