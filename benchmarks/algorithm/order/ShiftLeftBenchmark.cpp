#include <raze/algorithm/order/ShiftLeft.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_shift_left(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto src = rb::make_iota<T>(size);
    const auto shift = static_cast<std::ptrdiff_t>(size / 3);
    rb::bench_inplace_reset<T>(state, src, [&](auto& d) { return fn(d, shift); });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("ShiftLeft",
        [](benchmark::State& s) { run_shift_left<T>(s, [](auto& d, auto sh) { return raze::algorithm::shift_left(d, sh); }); },
        [](benchmark::State& s) { run_shift_left<T>(s, [](auto& d, auto sh) { return std::shift_left(d.begin(), d.end(), sh); }); },
        [](benchmark::State& s) { run_shift_left<T>(s, [](auto& d, auto sh) { return raze::algorithm::shift_left[raze::options::fscalar](d, sh); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

