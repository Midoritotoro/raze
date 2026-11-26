#include <raze/algorithm/set/Includes.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_includes(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto a = rb::make_sorted<T>(size);
    std::vector<T> sub;
    sub.reserve(size / 2 + 1);
    for (std::size_t i = 0; i < size; i += 2)
        sub.push_back(a[i]);
    rb::bench_binary_read<T>(state, a, sub, std::forward<Fn>(fn), 2);
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Includes",
        [](benchmark::State& s) { run_includes<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::includes(a, b); }); },
        [](benchmark::State& s) { run_includes<T>(s, [](const auto& a, const auto& b) { return std::ranges::includes(a, b); }); },
        [](benchmark::State& s) { run_includes<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::includes[raze::options::fscalar](a, b); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

