#include <raze/algorithm/order/IsPermutation.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

inline constexpr std::array<std::int64_t, 4> perm_sizes{ 16, 64, 256, 1024 };

template <class T, class Fn>
static void run_is_permutation(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto a = rb::make_unsorted<T>(size);
    auto b = a;
    std::mt19937 rng(42);
    std::ranges::shuffle(b, rng);
    rb::bench_binary_read<T>(state, a, b, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("IsPermutation",
        [](benchmark::State& s) { run_is_permutation<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::is_permutation(a, b); }); },
        [](benchmark::State& s) { run_is_permutation<T>(s, [](const auto& a, const auto& b) { return std::ranges::is_permutation(a, b); }); },
        [](benchmark::State& s) { run_is_permutation<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::is_permutation[raze::options::fscalar](a, b); }); },
        perm_sizes);
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

