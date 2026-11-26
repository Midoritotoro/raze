#include <raze/algorithm/find/Count.h>
#include <raze/algorithm/find/CountIf.h>
#include <raze/algorithm/find/CountIfNot.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_count(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_half_match<T>(rb::arg_size(state));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Count",
        [](benchmark::State& s) { run_count<T>(s, [](const auto& d) { return raze::algorithm::count(d, rb::needle_value<T>()); }); },
        [](benchmark::State& s) { run_count<T>(s, [](const auto& d) { return std::ranges::count(d, rb::needle_value<T>()); }); },
        [](benchmark::State& s) { run_count<T>(s, [](const auto& d) { return raze::algorithm::count[raze::options::fscalar](d, rb::needle_value<T>()); }); });

    rb::register_triplet<T>("CountPred",
        [](benchmark::State& s) { run_count<T>(s, [](const auto& d) { return raze::algorithm::count_if(d, rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_count<T>(s, [](const auto& d) { return std::ranges::count_if(d, rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_count<T>(s, [](const auto& d) { return raze::algorithm::count_if[raze::options::fscalar](d, rb::unary_pred<T>()); }); });

    rb::register_triplet<T>("CountIfNot",
        [](benchmark::State& s) { run_count<T>(s, [](const auto& d) { return raze::algorithm::count_if_not(d, rb::unary_pred_not<T>()); }); },
        [](benchmark::State& s) { run_count<T>(s, [](const auto& d) { return std::ranges::count_if(d, std::not_fn(rb::unary_pred_not<T>())); }); },
        [](benchmark::State& s) { run_count<T>(s, [](const auto& d) { return raze::algorithm::count_if_not[raze::options::fscalar](d, rb::unary_pred_not<T>()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();