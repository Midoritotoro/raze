#include <raze/algorithm/find/Find.h>
#include <raze/algorithm/find/FindIf.h>
#include <raze/algorithm/find/FindIfNot.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_find(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_find_data<T>(rb::arg_size(state, 0), rb::arg_size(state, 1));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Find",
        [](benchmark::State& s) { run_find<T>(s, [](const auto& d) { return raze::algorithm::find(d, rb::needle_value<T>()); }); },
        [](benchmark::State& s) { run_find<T>(s, [](const auto& d) { return std::ranges::find(d, rb::needle_value<T>()); }); },
        [](benchmark::State& s) { run_find<T>(s, [](const auto& d) { return raze::algorithm::find[raze::options::fscalar](d, rb::needle_value<T>()); }); },
        rb::end_pos_cases);

    rb::register_triplet<T>("FindPred",
        [](benchmark::State& s) { run_find<T>(s, [](const auto& d) { return raze::algorithm::find_if(d, rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_find<T>(s, [](const auto& d) { return std::ranges::find_if(d, rb::unary_pred<T>()); }); },
        [](benchmark::State& s) { run_find<T>(s, [](const auto& d) { return raze::algorithm::find_if[raze::options::fscalar](d, rb::unary_pred<T>()); }); },
        rb::end_pos_cases);

    rb::register_triplet<T>("FindIfNot",
        [](benchmark::State& s) { run_find<T>(s, [](const auto& d) { return raze::algorithm::find_if_not(d, rb::unary_pred_not<T>()); }); },
        [](benchmark::State& s) { run_find<T>(s, [](const auto& d) { return std::ranges::find_if_not(d, rb::unary_pred_not<T>()); }); },
        [](benchmark::State& s) { run_find<T>(s, [](const auto& d) { return raze::algorithm::find_if_not[raze::options::fscalar](d, rb::unary_pred_not<T>()); }); },
        rb::end_pos_cases);
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();