#include <raze/algorithm/order/Rotate.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_rotate(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    auto data = rb::make_iota<T>(size);
    const auto shift = static_cast<std::ptrdiff_t>(size / 3);
    rb::bench_inplace<T>(state, data, [&](auto& d) { return fn(d, d.begin() + shift); });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("Rotate",
        [](benchmark::State& s) { run_rotate<T>(s, [](auto& d, auto mid) { return raze::algorithm::rotate(d, mid); }); },
        [](benchmark::State& s) { run_rotate<T>(s, [](auto& d, auto mid) { return std::ranges::rotate(d, mid); }); },
        [](benchmark::State& s) { run_rotate<T>(s, [](auto& d, auto mid) { return raze::algorithm::rotate[raze::options::fscalar](d, mid); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();