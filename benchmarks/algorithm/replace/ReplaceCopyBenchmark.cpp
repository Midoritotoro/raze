#include <raze/algorithm/replace/ReplaceCopy.h>
#include <raze/algorithm/replace/ReplaceCopyIf.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_replace_copy(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_half_match<T>(rb::arg_size(state));
    rb::bench_copy<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("ReplaceCopy",
        [](benchmark::State& s) { run_replace_copy<T>(s, [](const auto& in, auto& out) { return raze::algorithm::replace_copy(in, out.begin(), rb::needle_value<T>(), rb::alt_value<T>()); }); },
        [](benchmark::State& s) { run_replace_copy<T>(s, [](const auto& in, auto& out) { return std::ranges::replace_copy(in, out.begin(), rb::needle_value<T>(), rb::alt_value<T>()); }); },
        [](benchmark::State& s) { run_replace_copy<T>(s, [](const auto& in, auto& out) { return raze::algorithm::replace_copy[raze::options::fscalar](in, out.begin(), rb::needle_value<T>(), rb::alt_value<T>()); }); });

    rb::register_triplet<T>("ReplaceCopyPred",
        [](benchmark::State& s) { run_replace_copy<T>(s, [](const auto& in, auto& out) { return raze::algorithm::replace_copy_if(in, out.begin(), rb::unary_pred<T>(), rb::alt_value<T>()); }); },
        [](benchmark::State& s) { run_replace_copy<T>(s, [](const auto& in, auto& out) { return std::ranges::replace_copy_if(in, out.begin(), rb::unary_pred<T>(), rb::alt_value<T>()); }); },
        [](benchmark::State& s) { run_replace_copy<T>(s, [](const auto& in, auto& out) { return raze::algorithm::replace_copy_if[raze::options::fscalar](in, out.begin(), rb::unary_pred<T>(), rb::alt_value<T>()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();