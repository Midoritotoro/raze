#include <raze/algorithm/fill/GenerateRandom.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_generate_random_urbg(benchmark::State& state, Fn&& fn) {
    std::vector<T> data(rb::arg_size(state));
    std::mt19937 rng(42);
    rb::bench_inplace<T>(state, data, [&](auto& d) { return fn(d, rng); });
}

template <class T, class Fn>
static void run_generate_random_dist(benchmark::State& state, Fn&& fn) {
    std::vector<T> data(rb::arg_size(state));
    std::mt19937 rng(42);
    if constexpr (std::is_floating_point_v<T>) {
        std::uniform_real_distribution<T> dist(T(-100), T(100));
        rb::bench_inplace<T>(state, data, [&](auto& d) { return fn(d, rng, dist); });
    }
    else {
        std::uniform_int_distribution<int> dist(-100, 100);
        rb::bench_inplace<T>(state, data, [&](auto& d) { return fn(d, rng, dist); });
    }
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("GenerateRandom",
        [](benchmark::State& s) { run_generate_random_urbg<T>(s, [](auto& d, auto& g) { return raze::algorithm::generate_random(d, g); }); },
        [](benchmark::State& s) { run_generate_random_urbg<T>(s, [](auto& d, auto& g) {
#if defined(__cpp_lib_ranges_generate_random) && __cpp_lib_ranges_generate_random >= 202403L
            return std::ranges::generate_random(d, g);
#else
            return std::ranges::generate(d, [&g]() { return static_cast<T>(g()); });
#endif
        }); },
        [](benchmark::State& s) { run_generate_random_urbg<T>(s, [](auto& d, auto& g) { return raze::algorithm::generate_random[raze::options::fscalar](d, g); }); });

    rb::register_triplet<T>("GenerateRandomDist",
        [](benchmark::State& s) { run_generate_random_dist<T>(s, [](auto& d, auto& g, auto& dist) { return raze::algorithm::generate_random(d, g, dist); }); },
        [](benchmark::State& s) { run_generate_random_dist<T>(s, [](auto& d, auto& g, auto& dist) {
#if defined(__cpp_lib_ranges_generate_random) && __cpp_lib_ranges_generate_random >= 202403L
            return std::ranges::generate_random(d, g, dist);
#else
            return std::ranges::generate(d, [&g, &dist]() { return static_cast<T>(dist(g)); });
#endif
        }); },
        [](benchmark::State& s) { run_generate_random_dist<T>(s, [](auto& d, auto& g, auto& dist) { return raze::algorithm::generate_random[raze::options::fscalar](d, g, dist); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

