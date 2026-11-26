#include <raze/algorithm/order/RotateCopy.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_rotate_copy(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto src = rb::make_iota<T>(size);
    const auto shift = static_cast<std::ptrdiff_t>(size / 3);
    rb::bench_copy<T>(state, src, [&](const auto& in, auto& out) { return fn(in, in.begin() + shift, out.begin()); });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("RotateCopy",
        [](benchmark::State& s) { run_rotate_copy<T>(s, [](const auto& in, auto mid, auto out) { return raze::algorithm::rotate_copy(in, mid, out); }); },
        [](benchmark::State& s) { run_rotate_copy<T>(s, [](const auto& in, auto mid, auto out) { return std::ranges::rotate_copy(in, mid, out); }); },
        [](benchmark::State& s) { run_rotate_copy<T>(s, [](const auto& in, auto mid, auto out) { return raze::algorithm::rotate_copy[raze::options::fscalar](in, mid, out); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();