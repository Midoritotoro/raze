#include <raze/algorithm/heap/SortHeap.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_sort_heap(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_heap_data<T>(rb::arg_size(state));
    rb::bench_inplace_reset<T>(state, src, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("SortHeap",
        [](benchmark::State& s) { run_sort_heap<T>(s, [](auto& d) { return raze::algorithm::sort_heap(d); }); },
        [](benchmark::State& s) { run_sort_heap<T>(s, [](auto& d) { return std::ranges::sort_heap(d); }); },
        [](benchmark::State& s) { run_sort_heap<T>(s, [](auto& d) { return raze::algorithm::sort_heap[raze::options::fscalar](d); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

