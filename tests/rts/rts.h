#pragma once

#include <raze/vx/Simd.h>
#include <raze/vx/Abi.h>

#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <type_traits>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <concepts>
#include <random>
#include <tuple>
#include <deque>
#include <list>
#include <forward_list>
#include <array>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <functional>
#include <memory>
#include <ranges>

#define RAZE_TEST_NAMESPACE_BEGIN namespace RAZE_TEST_ARCH_NAMESPACE {
#define RAZE_TEST_NAMESPACE_END }

namespace rtts {
    template <class ... Ts>
    struct types {
        template <class ... Us> 
        constexpr types<Ts..., Us...> operator+(types<Us...> const&) const;
    };

    template <class ... Ls> 
    struct concatenate { 
        using type = decltype((Ls{} + ...)); 
    };

    template <class ... Ls> 
    using concatenate_t = typename concatenate<Ls...>::type;

    template <class T > 
    struct type {};

    namespace detail {
        template <class T>
        constexpr inline auto to_bits(T const& value) noexcept {
            if constexpr (sizeof(T) == 1) return raze::math::bit_cast<raze::u8>(value);
            else if constexpr (sizeof(T) == 2) return raze::math::bit_cast<raze::u16>(value);
            else if constexpr (sizeof(T) == 4) return raze::math::bit_cast<raze::u32>(value);
            else if constexpr (sizeof(T) == 8) return raze::math::bit_cast<raze::u64>(value);
            else static_assert(sizeof(T) <= 8, "rtts::to_bits: unsupported type size");
        }

        template <class T>
        inline std::string bits_to_string(T const& value) {
            auto bits = to_bits(value);
            std::ostringstream os;
            os << "0x" << std::hex << std::setfill('0')
                << std::setw(sizeof(T) * 2) << bits;
            return os.str();
        }

        template <class T> 
        struct typename_impl {
            static auto value() noexcept {
#if defined(raze_cpp_msvc_only)
                std::string_view data(__FUNCSIG__);
                auto i = data.find('<') + 1, j = data.find(">::value");
                return std::string(data.substr(i, j - i));
#else
                std::string_view data(__PRETTY_FUNCTION__);
                auto i = data.find('=') + 2, j = data.find_last_of(']');
                return std::string(data.substr(i, j - i));
#endif
            }
        };
    }

    template <class T> 
    inline auto const typename_ = detail::typename_impl<T>::value();

    namespace detail {
        struct env {
            void pass() { 
                test_count++; 
                success_count++; 
            }

            void fail() { 
                test_count++; 
                failure_count++; 
            }

            void invalid() { 
                test_count++; 
                invalid_count++; 
            }
            
            int report() const {
                std::cout << "----------------------------------------------------------------\n";
                std::cout << "Results: " << test_count << " tests - "
                    << success_count << " successes - "
                    << failure_count << " failures - "
                    << invalid_count << " invalids\n";
                return (failure_count == 0 && invalid_count == 0) ? 0 : 1;
            }

            int test_count = 0, success_count = 0, failure_count = 0, invalid_count = 0;
            bool fail_status = false;
        };

        inline env global_runtime;
        inline std::string current_test = "";
        inline std::string current_type = "";

        struct logger {
            logger(bool status = true) : display(status), done(false) {}
            template <class Data> 
            logger& operator<<(Data const& d) {
                if (display) {
                    if (!done) { std::cout << ">> Additional information: \n"; done = true; }
                    std::cout << d;
                }
                return *this;
            }

            ~logger() noexcept { 
                if (display && done) std::cout << "\n"; 
            }
            
            bool display, done;
        };

        struct callable {
            using signature_t = void(*)(void*);
            signature_t invoker = nullptr;
            signature_t cleanup = nullptr;
            void* payload = nullptr;

            callable() = default;

            template <class Function>
            callable(Function f)
                : invoker{ invoke<Function> }
                , cleanup{ destroy<Function> }
                , payload{ new Function{std::move(f)} }
            {}

            callable(const callable&) = delete;
            callable& operator=(const callable&) = delete;

            callable(callable&& other) noexcept
                : invoker{ other.invoker }
                , cleanup{ other.cleanup }
                , payload{ other.payload }
            {
                other.invoker = nullptr;
                other.cleanup = nullptr;
                other.payload = nullptr;
            }

            callable& operator=(callable&& other) noexcept {
                if (this != &other) {
                    if (payload && cleanup) {
                        cleanup(payload);
                    }
                    invoker = other.invoker;
                    cleanup = other.cleanup;
                    payload = other.payload;

                    other.invoker = nullptr;
                    other.cleanup = nullptr;
                    other.payload = nullptr;
                }
                return *this;
            }

            ~callable() {
                if (payload && cleanup) {
                    cleanup(payload);
                    payload = nullptr;
                }
            }

            void operator()() {
                if (invoker && payload) {
                    invoker(payload);
                }
            }

            template <class T>
            static void invoke(void* data) {
                (*static_cast<T*>(data))();
            }

            template <class T>
            static void destroy(void* data) {
                delete static_cast<T*>(data);
            }
        };

        struct test {
            std::string name;
            callable behaviour;
            void operator()() { 
                current_test = name;
                behaviour(); 
            }
            static inline bool acknowledge(test&& f);
        };

        inline std::vector<test>& suite() {
            static std::vector<test> that = {};
            return that;
        }

        bool inline test::acknowledge(test&& f) {
            suite().emplace_back(std::forward<test>(f));
            return true;
        }

        struct test_capture {
            test_capture(const char* id) : name(id) {}
            auto operator+(auto body) const { return test::acknowledge({ name, body }); }
            const char* name;
        };

        template <class... Types>
        struct test_captures {
            test_captures(const char* id) : name(id) {}
            auto operator+(auto body) const {
                return test::acknowledge({
                    name,
                    [=]() {
                        (((current_type = " with [T = " + typename_<Types> +"]"), body(type<Types>())), ...);
                        current_type.clear();
                    }
                    });
            }
            std::string name;
        };

        template <class ... Types >
        struct test_captures<types<Types...>> : test_captures<Types...> {
            using test_captures<Types...>::test_captures;
        };
    }

    template <class T>
    inline std::string as_string(T const& e) {
        if constexpr (requires { std::to_string(e); }) return std::to_string(e);
        else if constexpr (requires(std::ostream & o) { o << e; }) { std::ostringstream os; os << e; return os.str(); }
        else return std::string("[") + typename_<T> +"]";
    }

    inline std::string as_string(bool b) { 
        return b ? "true" : "false"; 
    }

    template <class Simd> requires requires(Simd const& v) { Simd::size(); typename Simd::value_type; v[size_t{}]; }
    inline std::string as_string(Simd const& v) {
        std::ostringstream os;
        os << "{ ";
        for (size_t i = 0; i < Simd::size(); ++i) os << as_string(v[i]) << " ";
        os << "}";
        return os.str();
    }

#define PROTOTYPE(...) [] __VA_ARGS__

#define RTTS_CASE(ID) \
        [[maybe_unused]] static bool const RTTS_UNIQUE(rtts_case_) = \
        ::rtts::detail::test_capture{ID} + []()

#define RTTS_CASE_TPL(ID, ...) \
        [[maybe_unused]] static bool const RTTS_UNIQUE(rtts_case_) = \
        ::rtts::detail::test_captures<__VA_ARGS__>{ID} + PROTOTYPE()

#define RTTS_FAIL(Message) \
        do { \
            ::rtts::detail::global_runtime.fail(); \
            if (!::rtts::detail::global_runtime.fail_status) { \
                ::rtts::detail::global_runtime.fail_status = true; \
                std::cout << "[X] - " << ::rtts::detail::current_test << "\n"; \
            } \
            if (!::rtts::detail::current_type.empty()) std::cout << "  > " << ::rtts::detail::current_type << "\n"; \
            std::cout << "    [FAILURE] : " << Message << std::endl; \
        } while(0)

#define RTTS_EXPECT(EXPR) \
        [&](auto&& local_rtts_expr) { \
            if (local_rtts_expr) { ::rtts::detail::global_runtime.pass(); return ::rtts::detail::logger{false}; } \
            else { RTTS_FAIL("Expression: " << #EXPR << " evaluates to false."); return ::rtts::detail::logger{}; } \
        }(EXPR)

#define RTTS_EQUAL(LHS, RHS) \
        [&](auto&& a, auto&& b) { \
            if (a == b) { ::rtts::detail::global_runtime.pass(); return ::rtts::detail::logger{false}; } \
            else { RTTS_FAIL("Expected: " << #LHS << " == " << #RHS << " but " << ::rtts::as_string(a) << " != " << ::rtts::as_string(b)); return ::rtts::detail::logger{}; } \
        }(LHS, RHS)

#define RTTS_ALL_EQUAL(LHS, RHS) \
        [&](auto const& a, auto const& b) { \
            if (!std::ranges::equal(a, b)) { RTTS_FAIL("Expected: " << #LHS << " == " << #RHS << " but containers differ."); return ::rtts::detail::logger{}; } \
            else { ::rtts::detail::global_runtime.pass(); return ::rtts::detail::logger{false}; } \
        }(LHS, RHS)

#define RTTS_ALL_VALIDATE(LHS, GEN) \
        [&](auto const& a, auto const& b) { \
            bool ok = true; \
            size_t i = 0; \
            for(auto const& elem : a) { if(elem != b(i)) { ok = false; break; } ++i; } \
            if(ok) { ::rtts::detail::global_runtime.pass(); return ::rtts::detail::logger{false}; } \
            else { RTTS_FAIL("Expected: " << #LHS << " == " << #GEN << " but vectors differ."); return ::rtts::detail::logger{}; } \
        }(LHS, GEN)

#define RTTS_ALL_VALIDATE_BITS(LHS, GEN) \
    [&](auto const& rtts_a, auto const& rtts_gen) { \
        bool rtts_ok = true; \
        size_t rtts_bad = 0; \
        size_t rtts_i = 0; \
        auto rtts_bad_val = typename std::decay_t<decltype(rtts_a)>::value_type{}; \
        for(auto const& elem : rtts_a) { \
            if(::rtts::detail::to_bits(elem) != ::rtts::detail::to_bits(rtts_gen(rtts_i))) { \
                rtts_ok = false; rtts_bad = rtts_i; rtts_bad_val = elem; break; \
            } \
            ++rtts_i; \
        } \
        if(rtts_ok) { ::rtts::detail::global_runtime.pass(); return ::rtts::detail::logger{false}; } \
        else { \
            RTTS_FAIL("Expected bitwise: " << #LHS << " == " << #GEN \
                << " but differ at [" << rtts_bad << "]: " \
                << ::rtts::detail::bits_to_string(rtts_bad_val) \
                << " != " << ::rtts::detail::bits_to_string(rtts_gen(rtts_bad))); \
            return ::rtts::detail::logger{}; \
        } \
    }(LHS, GEN)

    namespace algorithm {
        using all_types = types<
            char, short, int, long long,
            unsigned char, unsigned short, unsigned int, unsigned long long,
            float, double
        >;
    } // namespace algorithm

    namespace random {
        template <class T>
        struct generator {
            std::mt19937 gen;

            explicit generator(unsigned seed = 42) : gen(seed) {}

            T operator()() {
                if constexpr (std::is_same_v<T, bool>) {
                    return (gen() & 1) != 0;
                } else if constexpr (std::is_integral_v<T>) {
                    if constexpr (std::is_signed_v<T>) {
                        std::uniform_int_distribution<long long> dist(-1000, 1000);
                        return static_cast<T>(dist(gen));
                    } else {
                        std::uniform_int_distribution<unsigned long long> dist(0, 2000);
                        return static_cast<T>(dist(gen));
                    }
                } else if constexpr (std::is_floating_point_v<T>) {
                    std::uniform_real_distribution<T> dist(T(-1000), T(1000));
                    return dist(gen);
                } else {
                    static_assert(std::is_arithmetic_v<T>, "Unsupported random type");
                }
            }
        };

        template <template <class...> class Container, class T>
        Container<T> sequence(size_t size, unsigned seed = 42) {
            generator<T> gen(seed);

            Container<T> result(size);
            for (auto& x : result)
                x = gen();

            return result;
        }

        template <class Container>
        Container sequence(size_t size, unsigned seed = 42) {
            using T = typename Container::value_type;
            generator<T> gen(seed);

            Container result(size);
            for (auto& x : result)
                x = gen();

            return result;
        }

        template <class T>
        std::vector<T> vector(size_t size, unsigned seed = 42) {
            return sequence<std::vector, T>(size, seed);
        }

        template <class T>
        std::deque<T> deque(size_t size, unsigned seed = 42) {
            return sequence<std::deque, T>(size, seed);
        }

        template <class T>
        std::list<T> list(size_t size, unsigned seed = 42) {
            return sequence<std::list, T>(size, seed);
        }

        template <class T>
        std::forward_list<T> forward_list(size_t size, unsigned seed = 42) {
            return sequence<std::forward_list, T>(size, seed);
        }

        template <class T, size_t N>
        std::array<T, N> array(unsigned seed = 42) {
            generator<T> gen(seed);

            std::array<T, N> result;
            for (auto& x : result)
                x = gen();

            return result;
        }
    } // namespace random

    namespace simd {
        constexpr raze::arch::ISA current_isa() {
            return raze::vx::target_isa();
        }

        template <class Mask>
        raze_always_inline Mask make_alternating_mask() {
            Mask m(true);

            for (size_t i = 0; i < Mask::size(); ++i)
               m[i] = (i % 2) == 0;

            return m;
        }

        template <class Mask>
        raze_always_inline Mask make_random_mask() {
            Mask m;
            static std::mt19937_64 rng(0x123456789ABCDEFULL);

            for (size_t i = 0; i < Mask::size(); ++i)
                m[i] = (rng() & 1) != 0;

            return m;
        }

        template <class T, raze::u32 Width>
        struct simd_info {
            using base_type = T;
            static constexpr raze::u32 width = Width;
            using type = raze::vx::simd<base_type, raze::vx::x86_abi<width / (sizeof(T) * 8)>>;
        };

        template <class T>
        struct widths_for_type {
            using type = types<
                simd_info<T, raze_sizeof_in_bits(T)>,
                simd_info<T, raze::vx::simd<T>::size() * raze_sizeof_in_bits(T)>
#if (RAZE_HAS_SSE2_SUPPORT && !RAZE_HAS_AVX512F_SUPPORT) || RAZE_HAS_AVX512VL_SUPPORT
                ,simd_info<T, 128>
#endif
#if RAZE_HAS_AVX512VL_SUPPORT || RAZE_HAS_AVX_SUPPORT
                ,simd_info<T, 256>
#endif
#if RAZE_HAS_AVX512F_SUPPORT
                ,simd_info<T, 512>
#endif  
            >;
        };

        template <class ... Lists>
        struct concatenate_lists;

        template <>
        struct concatenate_lists<> {
            using type = types<>;
        };

        template <class ... Ts, class ... Rest>
        struct concatenate_lists<types<Ts...>, Rest...> {
            using type = concatenate_t<types<Ts...>, typename concatenate_lists<Rest...>::type>;
        };

        template <class... Ts>
        auto make_all_widths(types<Ts...>) {
            return typename concatenate_lists<typename widths_for_type<Ts>::type...>::type{};
        }

        using base_types = types<raze::i8, raze::u8, raze::i16, raze::u16, raze::i32, raze::u32, raze::i64, raze::u64, raze::f32, raze::f64>;
        using fp_types = types<raze::f32, raze::f64>;

        using all_simd_infos = decltype(make_all_widths(base_types{}));
        using all_fp_infos = decltype(make_all_widths(fp_types{}));

        template <class Info>
        struct is_valid_simd_info {
            static constexpr bool value = requires {
                typename raze::vx::simd<typename Info::base_type, raze::vx::runtime_abi<current_isa(), Info::width>>;
            };
        };

        template <template <class> class Pred, class Type> 
        struct filter {
            template <class... Ls> static types<Ls...> tuple_to_types(const std::tuple<Ls...>&);
            template <class T>
            static std::conditional_t<Pred<T>::value, std::tuple<T>, std::tuple<>> filter_type();
            template <class... Ls> static auto filter_impl(types<Ls...>) {
                return tuple_to_types(std::tuple_cat(filter_type<Ls>()...));
            }

            using type = decltype(filter_impl(Type{}));
        };

        using all_types = typename filter<is_valid_simd_info, all_simd_infos>::type;
        using all_fp_types = typename filter<is_valid_simd_info, all_fp_infos>::type;
    }

    namespace algorithm {
        enum class kind { range, n };

        inline constexpr kind range = kind::range;
        inline constexpr kind n = kind::n;

        struct config {
            size_t random_cases = 256;
            size_t random_max_size = 1000;
            unsigned seed = 42;
            bool counted = true;
            std::vector<size_t> sizes = {
                0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32,
                63, 64, 127, 128, 255, 256
            };

            static config thorough() {
                config result;
                result.random_cases = 1000;
                result.sizes = {
                    0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32,
                    63, 64, 127, 128, 255, 256,
                    10000, 50000, 100000
                };
                return result;
            }

            config& random(size_t cases, size_t max_size) {
                random_cases = cases;
                random_max_size = max_size;
                return *this;
            }

            config& add_sizes(std::initializer_list<size_t> values) {
                sizes.insert(sizes.end(), values.begin(), values.end());
                return *this;
            }

            config& counted_only(bool value = true) {
                counted = value;
                return *this;
            }
        };

        template <class T, class Container>
        struct fixture {
            Container value;
            Container second;
            std::vector<T> output;
            std::vector<T> output2;
            std::vector<T> pattern;
            size_t pattern_size = 0;
            size_t count = 0;
            T value_arg{};
            T other_arg = T(17);
            T replacement = T(42);

            size_t size() const { return static_cast<size_t>(std::ranges::distance(value)); }
        };

        template <bool Counted, class Fixture>
        struct args {
            Fixture& data;

            constexpr auto first() const {
                if constexpr (Counted)
                    return std::counted_iterator(data.value.begin(), static_cast<std::ptrdiff_t>(data.size()));
                else
                    return data.value.begin();
            }

            constexpr auto last() const {
                if constexpr (Counted)
                    return std::default_sentinel;
                else
                    return data.value.end();
            }

            constexpr auto first2() const {
                if constexpr (Counted)
                    return std::counted_iterator(data.second.begin(), static_cast<std::ptrdiff_t>(std::ranges::distance(data.second)));
                else
                    return data.second.begin();
            }

            constexpr auto last2() const {
                if constexpr (Counted)
                    return std::default_sentinel;
                else
                    return data.second.end();
            }

            constexpr auto out() const {
                return data.output.begin();
            }

            constexpr auto out2() const {
                return data.output2.begin();
            }

            constexpr auto pattern_first() const {
                return data.pattern.begin();
            }

            constexpr auto pattern_last() const {
                return data.pattern.begin() + data.pattern_size;
            }

            constexpr size_t count() const {
                return data.count;
            }

            constexpr auto& value() const {
                return data.value_arg;
            }

            constexpr auto& other() const {
                return data.other_arg;
            }

            constexpr auto& replacement() const {
                return data.replacement;
            }
        };

        template <class It>
        constexpr auto base(It it) {
            if constexpr (requires { it.base(); }) return it.base();
            else return it;
        }

        template <class Container, class It>
        constexpr size_t position(Container const& value, It it) {
            if constexpr (std::is_same_v<std::decay_t<It>, std::default_sentinel_t>) {
                return static_cast<size_t>(std::ranges::distance(value));
            } else {
                return static_cast<size_t>(std::ranges::distance(value.begin(), base(it)));
            }
        }

        template <class A, class B>
        constexpr bool iterator_equal(A const& a, B const& b, auto const& ca, auto const& cb) {
            return position(ca, a) == position(cb, b);
        }

        template <class A, class B>
        constexpr bool input2_equal(A const& a, B const& b, auto const& ca, auto const& cb) {
            return position(ca.second, a) == position(cb.second, b);
        }

        template <class A, class B>
        constexpr bool output_equal(A const& a, B const& b, auto const& ca, auto const& cb) {
            return position(ca.output, a) == position(cb.output, b);
        }

        template <class A, class B>
        constexpr bool output2_equal(A const& a, B const& b, auto const& ca, auto const& cb) {
            return position(ca.output2, a) == position(cb.output2, b);
        }

        template <class A, class B>
        constexpr bool result_equal(A const& a, B const& b, auto const& ca, auto const& cb) {
            bool handled = false;
            bool result = true;

            if constexpr (requires { a.in; b.in; }) {
                handled = true;
                result &= iterator_equal(a.in, b.in, ca.value, cb.value);
            }

            if constexpr (requires { a.out; b.out; }) {
                handled = true;
                result &= output_equal(a.out, b.out, ca, cb);
            }

            if constexpr (requires { a.in1; b.in1; }) {
                handled = true;
                result &= iterator_equal(a.in1, b.in1, ca.value, cb.value);
            }
            if constexpr (requires { a.in2; b.in2; }) {
                handled = true;
                result &= input2_equal(a.in2, b.in2, ca, cb);
            }
            if constexpr (requires { a.out1; b.out1; }) {
                handled = true;
                result &= output_equal(a.out1, b.out1, ca, cb);
            }
            if constexpr (requires { a.out2; b.out2; }) {
                handled = true;
                result &= output2_equal(a.out2, b.out2, ca, cb);
            }

            if constexpr (requires { a.min; b.min; a.max; b.max; }) {
                handled = true;
                result &= iterator_equal(a.min, b.min, ca.value, cb.value);
                result &= iterator_equal(a.max, b.max, ca.value, cb.value);
            }

            if constexpr (requires { a.begin(); a.end(); b.begin(); b.end(); }) {
                if (!handled) {
                    handled = true;
                    result &= iterator_equal(a.begin(), b.begin(), ca.value, cb.value);
                    result &= iterator_equal(a.end(), b.end(), ca.value, cb.value);
                }
            }

            if (!handled) {
                if constexpr (std::input_or_output_iterator<A> && std::input_or_output_iterator<B>)
                    result = iterator_equal(a, b, ca.value, cb.value);
                else if constexpr (std::equality_comparable_with<A, B>)
                    result = a == b;
                else
                    result = false;  
            }

            return result;
        }

        template <class A, class B>
        constexpr bool state_equal(A const& a, B const& b) {
            return std::ranges::equal(a.value, b.value) &&
                std::ranges::equal(a.second, b.second) &&
                std::ranges::equal(a.output, b.output) &&
                std::ranges::equal(a.output2, b.output2);
        }

        template <class Result>
        constexpr auto logical_end(Result const& result) {
            if constexpr (requires { result.in; })
                return base(result.in);
            else if constexpr (requires { result.begin(); })
                return base(result.begin());
            else
                return base(result);
        }

        struct same {
            template <class A, class B, class RA, class RB>
            constexpr bool operator()(A const& a, B const& b, RA const& ra, RB const& rb) const {
                return state_equal(a, b) && result_equal(ra, rb, a, b);
            }
        };

        struct prefix {
            template <class A, class B, class RA, class RB>
            constexpr bool operator()(A const& a, B const& b, RA const& ra, RB const& rb) const {
                const auto ae = logical_end(ra);
                const auto be = logical_end(rb);

                return std::ranges::equal(
                    std::ranges::subrange(a.value.begin(), ae),
                    std::ranges::subrange(b.value.begin(), be)) &&
                    result_equal(ra, rb, a, b);
            }
        };

        inline constexpr same same_result{};
        inline constexpr prefix prefix_result{};

        template <class T, class F>
        void each_container(size_t size, unsigned seed, F f) {
            f(rtts::random::vector<T>(size, seed));
            f(rtts::random::deque<T>(size, seed));
            f(rtts::random::list<T>(size, seed));
            f(rtts::random::forward_list<T>(size, seed));
        }

        template <class F>
        void each_count(size_t size, std::mt19937& rng, bool boundary, F f) {
            if (size == 0)
                return;

            if (boundary) {
                f(size_t(0));
                if (size > 1) f(size / 2);
                if (size > 2 && size - 1 != size / 2) f(size - 1);
            }
            else {
                f(rng() % size);
            }
        }

        template <bool Counted, class Fixture, class Raze, class Std, class Verify>
        constexpr bool variant(Fixture& raze_data, Fixture& std_data, Raze& raze_op, Std& std_op, Verify& verify) {
            if constexpr (Counted) {
                if (raze_data.size() == 0)
                    return true;
            }

            const args<Counted, Fixture> raze_args{ raze_data };
            const args<Counted, Fixture> std_args{ std_data };

            const auto raze_ret = raze_op(raze_args);
            const auto std_ret = std_op(std_args);

            return verify(raze_data, std_data, raze_ret, std_ret);
        }

        template <class T, class Container>
        auto make_fixture_from_container(Container c, unsigned seed) {
            fixture<T, Container> f;
            f.value = std::move(c);

            const size_t sz = f.size();
            f.second = rtts::random::sequence<Container>(sz, seed + 1);

            f.output.resize(sz);
            f.output2.resize(sz);

            size_t pat_sz = std::min<size_t>(3, sz);
            f.pattern.resize(pat_sz);
            std::copy_n(f.value.begin(), pat_sz, f.pattern.begin());
            f.pattern_size = pat_sz;

            f.value_arg = f.value.empty() ? T{} : f.value.front();
            f.other_arg = T(17);
            f.replacement = T(42);

            return f;
        }

        template <kind Kind, class T, class Container, class Raze, class Std, class Verify>
        void case_(Container value, unsigned seed, std::mt19937& rng, bool boundary,
            Raze& raze_op, Std& std_op, Verify& verify) {

            auto raze_data = make_fixture_from_container<T>(std::move(value), seed);
            auto std_data = raze_data;  

            auto run = [&](size_t count) {
                raze_data.count = count;
                std_data.count = count;

                RTTS_EXPECT(variant<false>(
                    raze_data, std_data, raze_op, std_op, verify));

                if (raze_data.size() != 0)
                    RTTS_EXPECT(variant<true>(raze_data, std_data, raze_op, std_op, verify));
                };

            if constexpr (Kind == kind::range) {
                run(0);  
            }
            else {
                each_count(raze_data.size(), rng, boundary, run);
            }
        }

        template <kind Mode, class T, class Raze, class Std, class Verify = same>
        void run(config cfg, Raze raze_op, Std std_op, Verify verify = {}) {
            std::mt19937 rng(cfg.seed);
            std::uniform_int_distribution<size_t> size_dist(0, cfg.random_max_size);

            for (size_t i = 0; i < cfg.random_cases; ++i) {
                const size_t size = size_dist(rng);
                const unsigned seed = cfg.seed + static_cast<unsigned>(i);

                each_container<T>(size, seed, [&](auto value) {
                    case_<Mode == kind::n ? kind::n : kind::range, T>(
                        std::move(value), seed, rng, false, raze_op, std_op, verify);
                    });
            }

            for (size_t i = 0; i < cfg.sizes.size(); ++i) {
                const size_t size = cfg.sizes[i];
                const unsigned seed = cfg.seed + 100000 + static_cast<unsigned>(i);

                each_container<T>(size, seed, [&](auto value) {
                    case_<Mode == kind::n ? kind::n : kind::range, T>(
                        std::move(value), seed, rng, true, raze_op, std_op, verify);
                    });
            }
        }

        template <class T, size_t N>
        struct constexpr_fixture {
            std::array<T, N> value{};
            std::array<T, N> second{};
            std::array<T, N> output{};
            std::array<T, N> output2{};
            std::array<T, N> pattern{};
            size_t pattern_size = N < 3 ? N : 3;
            size_t count = 0;
            T value_arg{};
            T other_arg = T(17);
            T replacement = T(42);

            constexpr size_t size() const {
                return N;
            }
        };

        template <class T, size_t N>
        constexpr auto make_constexpr_fixture() {
            constexpr_fixture<T, N> result{};
            for (size_t i = 0; i < N; ++i) {
                result.value[i] = T(i + 1);
                result.second[i] = T(100 + i);
                result.output[i] = T(200 + i);
                result.output2[i] = T(300 + i);
                result.pattern[i] = result.value[i];
            }
            if constexpr (N != 0)
                result.value_arg = result.value[0];
            return result;
        }

        template <class T, kind Kind, size_t Size, size_t Count = 0, class Raze, class Std, class Verify = same>
        consteval bool constexpr_run(Raze raze_op, Std std_op, Verify verify = {}) {
            if constexpr (Kind == kind::n)
                static_assert(Size != 0 && Count < Size);

            auto raze_data = make_constexpr_fixture<T, Size>();
            auto std_data = raze_data;

            raze_data.count = Count;
            std_data.count = Count;

            if (!variant<false>(
                raze_data, std_data, raze_op, std_op, verify))
                return false;

            if constexpr (Size != 0) {
                if (!variant<true>(
                    raze_data, std_data, raze_op, std_op, verify))
                    return false;
            }

            return true;
        }
    }

    inline int run() {
        std::size_t done_tests = 0;
        bool aborted = false;

        try {
            for (auto& t : detail::suite()) {
                auto test_count = detail::global_runtime.test_count;
                auto failure_count = detail::global_runtime.failure_count;

                detail::global_runtime.fail_status = false;

                t();

                done_tests++;

                if (test_count == detail::global_runtime.test_count) {
                    detail::global_runtime.invalid();

                    std::cout
                        << "[!] - "
                        << detail::current_test
                        << " : EMPTY TEST CASE\n";
                }
            }
        }
        catch (...) {
            aborted = true;

            std::cout
                << "@@ ABORTING DUE TO EXCEPTION @@ - "
                << (detail::suite().size() - done_tests - 1)
                << " Tests not run\n";
        }

        const int result = detail::global_runtime.report();

        if (aborted) {
            return 1;
        }

        return result;
    }
}

#define RTTS_UNIQUE3(ID, LINE) ID##LINE
#define RTTS_UNIQUE2(ID, LINE) RTTS_UNIQUE3(ID, LINE)
#define RTTS_UNIQUE(ID)        RTTS_UNIQUE2(ID, __COUNTER__)