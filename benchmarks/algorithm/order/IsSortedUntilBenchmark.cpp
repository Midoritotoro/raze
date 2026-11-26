#include <raze/algorithm/order/IsSortedUntil.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_is_sorted_until(benchmark::State& state, bool descending, Fn&& fn) {
    const auto data = rb::make_sorted_until<T>(rb::arg_size(state), descending);
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("IsSortedUntil",
        [](benchmark::State& s) { run_is_sorted_until<T>(s, false, [](const auto& d) { return raze::algorithm::is_sorted_until(d); }); },
        [](benchmark::State& s) { run_is_sorted_until<T>(s, false, [](const auto& d) { return std::ranges::is_sorted_until(d); }); },
        [](benchmark::State& s) { run_is_sorted_until<T>(s, false, [](const auto& d) { return raze::algorithm::is_sorted_until[raze::options::fscalar](d); }); });

    rb::register_triplet<T>("IsSortedUntilPred",
        [](benchmark::State& s) { run_is_sorted_until<T>(s, true, [](const auto& d) { return raze::algorithm::is_sorted_until(d, std::greater<>{}); }); },
        [](benchmark::State& s) { run_is_sorted_until<T>(s, true, [](const auto& d) { return std::ranges::is_sorted_until(d, std::greater<>{}); }); },
        [](benchmark::State& s) { run_is_sorted_until<T>(s, true, [](const auto& d) { return raze::algorithm::is_sorted_until[raze::options::fscalar](d, std::greater<>{}); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();