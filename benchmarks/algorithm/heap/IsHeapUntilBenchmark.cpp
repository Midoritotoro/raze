#include <raze/algorithm/heap/IsHeapUntil.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_is_heap_until(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_heap_data<T>(rb::arg_size(state));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("IsHeapUntil",
        [](benchmark::State& s) { run_is_heap_until<T>(s, [](const auto& d) { return raze::algorithm::is_heap_until(d); }); },
        [](benchmark::State& s) { run_is_heap_until<T>(s, [](const auto& d) { return std::ranges::is_heap_until(d); }); },
        [](benchmark::State& s) { run_is_heap_until<T>(s, [](const auto& d) { return raze::algorithm::is_heap_until[raze::options::fscalar](d); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

