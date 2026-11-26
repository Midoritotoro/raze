#include <raze/algorithm/order/PrevPermutation.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_prev_permutation(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    auto src = rb::make_sorted<T>(size, false);
    if (size >= 2) src[0] = static_cast<T>(120);
    rb::bench_inplace_reset<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("PrevPermutation",
        [](benchmark::State& s) { run_prev_permutation<T>(s, [](auto& d) { return raze::algorithm::prev_permutation(d); }); },
        [](benchmark::State& s) { run_prev_permutation<T>(s, [](auto& d) { return std::ranges::prev_permutation(d); }); },
        [](benchmark::State& s) { run_prev_permutation<T>(s, [](auto& d) { return raze::algorithm::prev_permutation[raze::options::fscalar](d); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

