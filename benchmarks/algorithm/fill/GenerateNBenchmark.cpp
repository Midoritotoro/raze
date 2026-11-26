#include <raze/algorithm/fill/GenerateN.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_generate_n(benchmark::State& state, Fn&& fn) {
    auto data = rb::make_iota<T>(rb::arg_size(state));
    std::uint32_t counter = 0;

    rb::bench_inplace<T>(state, data, [&](auto& d) {
        return fn(d.begin(), static_cast<std::ptrdiff_t>(d.size()), [&counter]() noexcept -> T {
            return static_cast<T>(++counter & 0x7Fu);
        });
    });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("GenerateN",
        [](benchmark::State& s) { run_generate_n<T>(s, [](auto it, auto n, auto gen) { return raze::algorithm::generate_n(it, n, gen); }); },
        [](benchmark::State& s) { run_generate_n<T>(s, [](auto it, auto n, auto gen) { return std::ranges::generate_n(it, n, gen); }); },
        [](benchmark::State& s) { run_generate_n<T>(s, [](auto it, auto n, auto gen) { return raze::algorithm::generate_n[raze::options::fscalar](it, n, gen); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

