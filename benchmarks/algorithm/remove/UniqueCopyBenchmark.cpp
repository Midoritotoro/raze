#include <raze/algorithm/remove/UniqueCopy.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_unique_copy(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_unique_data<T>(rb::arg_size(state));
    rb::bench_copy<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("UniqueCopy",
        [](benchmark::State& s) { run_unique_copy<T>(s, [](const auto& in, auto& out) { return raze::algorithm::unique_copy(in, out.begin()); }); },
        [](benchmark::State& s) { run_unique_copy<T>(s, [](const auto& in, auto& out) { return std::ranges::unique_copy(in, out.begin()); }); },
        [](benchmark::State& s) { run_unique_copy<T>(s, [](const auto& in, auto& out) { return raze::algorithm::unique_copy[raze::options::fscalar](in, out.begin()); }); });

    rb::register_triplet<T>("UniqueCopyPred",
        [](benchmark::State& s) { run_unique_copy<T>(s, [](const auto& in, auto& out) { return raze::algorithm::unique_copy(in, out.begin(), rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_unique_copy<T>(s, [](const auto& in, auto& out) { return std::ranges::unique_copy(in, out.begin(), rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_unique_copy<T>(s, [](const auto& in, auto& out) { return raze::algorithm::unique_copy[raze::options::fscalar](in, out.begin(), rb::binary_eq_pred<T>()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

