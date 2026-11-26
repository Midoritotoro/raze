#include <raze/algorithm/heap/PushHeap.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_push_heap(benchmark::State& state, Fn&& fn) {
    const auto src = rb::make_unsorted<T>(rb::arg_size(state));
    rb::bench_inplace_reset<T>(state, src, [&](auto& d) {
        const auto n = d.size();
        for (std::size_t i = 2; i <= n; ++i)
            fn(d.begin(), d.begin() + static_cast<std::ptrdiff_t>(i));
        return d.end();
    });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("PushHeap",
        [](benchmark::State& s) { run_push_heap<T>(s, [](auto f, auto l) { return raze::algorithm::push_heap(f, l); }); },
        [](benchmark::State& s) { run_push_heap<T>(s, [](auto f, auto l) { return std::ranges::push_heap(f, l); }); },
        [](benchmark::State& s) { run_push_heap<T>(s, [](auto f, auto l) { return raze::algorithm::push_heap[raze::options::fscalar](f, l); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

