#include <raze/algorithm/heap/PopHeap.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_pop_heap(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_heap_data<T>(rb::arg_size(state));
    rb::bench_inplace_reset<T>(state, src, [&](auto& d) {
        for (std::size_t len = d.size(); len > 1; --len)
            fn(d.begin(), d.begin() + static_cast<std::ptrdiff_t>(len));
        return d.end();
    });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("PopHeap",
        [](benchmark::State& s) { run_pop_heap<T>(s, [](auto f, auto l) { return raze::algorithm::pop_heap(f, l); }); },
        [](benchmark::State& s) { run_pop_heap<T>(s, [](auto f, auto l) { return std::ranges::pop_heap(f, l); }); },
        [](benchmark::State& s) { run_pop_heap<T>(s, [](auto f, auto l) { return raze::algorithm::pop_heap[raze::options::fscalar](f, l); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

