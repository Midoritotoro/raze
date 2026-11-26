#include <raze/algorithm/heap/MakeHeap.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_make_heap(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_unsorted<T>(rb::arg_size(state));
    rb::bench_inplace_reset<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("MakeHeap",
        [](benchmark::State& s) { run_make_heap<T>(s, [](auto& d) { return raze::algorithm::make_heap(d); }); },
        [](benchmark::State& s) { run_make_heap<T>(s, [](auto& d) { return std::ranges::make_heap(d); }); },
        [](benchmark::State& s) { run_make_heap<T>(s, [](auto& d) { return raze::algorithm::make_heap[raze::options::fscalar](d); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

