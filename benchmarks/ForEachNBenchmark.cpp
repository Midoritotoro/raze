#include <raze/algorithm/batch/ForEachN.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_for_each_n(benchmark::State& state, Fn&& fn) {
    auto data = rb::make_iota<T>(rb::arg_size(state));
    const auto n = static_cast<std::ptrdiff_t>(data.size());
    rb::bench_inplace<T>(state, data, [&](auto& d) { return fn(d, n); }, 2);
}

template <class T>
static void register_type() {
    auto op = [](auto& x) { x = (x * T(-1)) + T(100); };

    rb::register_triplet<T>("ForEachN",
        [=](benchmark::State& s) { run_for_each_n<T>(s, [=](auto& d, std::ptrdiff_t n) { return raze::algorithm::for_each_n(d.begin(), n, op); }); },
        [=](benchmark::State& s) { run_for_each_n<T>(s, [=](auto& d, std::ptrdiff_t n) { return std::ranges::for_each_n(d.begin(), n, op); }); },
        [=](benchmark::State& s) { run_for_each_n<T>(s, [=](auto& d, std::ptrdiff_t n) { return raze::algorithm::for_each_n[raze::options::fscalar](d.begin(), n, op); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

