#include <raze/algorithm/find/Mismatch.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_mismatch(benchmark::State& state, Fn&& fn) {
    const auto a = rb::make_iota<T>(rb::arg_size(state));
    const auto b = a;
    rb::bench_binary_read<T>(state, a, b, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Mismatch",
        [](benchmark::State& s) { run_mismatch<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::mismatch(a, b); }); },
        [](benchmark::State& s) { run_mismatch<T>(s, [](const auto& a, const auto& b) { return std::ranges::mismatch(a, b); }); },
        [](benchmark::State& s) { run_mismatch<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::mismatch[raze::options::fscalar](a, b); }); });

    rb::register_triplet<T>("MismatchPred",
        [](benchmark::State& s) { run_mismatch<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::mismatch(a, b, rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_mismatch<T>(s, [](const auto& a, const auto& b) { return std::ranges::mismatch(a, b, rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_mismatch<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::mismatch[raze::options::fscalar](a, b, rb::binary_eq_pred<T>()); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();