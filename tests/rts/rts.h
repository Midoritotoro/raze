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
        constexpr auto to_bits(T const& value) noexcept {
            if constexpr (sizeof(T) == 1) return raze::math::bit_cast<raze::u8>(value);
            else if constexpr (sizeof(T) == 2) return raze::math::bit_cast<raze::u16>(value);
            else if constexpr (sizeof(T) == 4) return raze::math::bit_cast<raze::u32>(value);
            else if constexpr (sizeof(T) == 8) return raze::math::bit_cast<raze::u64>(value);
            else static_assert(sizeof(T) <= 8, "rtts::to_bits: unsupported type size");
        }

        template <class T>
        std::string bits_to_string(T const& value) {
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
        struct test_captures<types<Types...>> : test_captures<Types...> {};
    }

    template <class T>
    std::string as_string(T const& e) {
        if constexpr (requires { std::to_string(e); }) return std::to_string(e);
        else if constexpr (requires(std::ostream & o) { o << e; }) { std::ostringstream os; os << e; return os.str(); }
        else return std::string("[") + typename_<T> +"]";
    }

    inline std::string as_string(bool b) { 
        return b ? "true" : "false"; 
    }

    template <class Simd> requires requires { Simd::size(); typename Simd::value_type; }
    std::string as_string(Simd const& v) {
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
            if (std::size(a) != std::size(b)) { RTTS_FAIL("Sizes don't match"); return ::rtts::detail::logger{}; } \
            bool ok = true; \
            for(size_t i=0; i<std::size(a); ++i) if(a[i] != b[i]) { ok = false; break; } \
            if(ok) { ::rtts::detail::global_runtime.pass(); return ::rtts::detail::logger{false}; } \
            else { RTTS_FAIL("Expected: " << #LHS << " == " << #RHS << " but vectors differ."); return ::rtts::detail::logger{}; } \
        }(LHS, RHS)

#define RTTS_ALL_VALIDATE(LHS, GEN) \
        [&](auto const& a, auto const& b) { \
            bool ok = true; \
            for(size_t i=0; i<std::size(a); ++i) if(a[i] != b(i)) { ok = false; break; } \
            if(ok) { ::rtts::detail::global_runtime.pass(); return ::rtts::detail::logger{false}; } \
            else { RTTS_FAIL("Expected: " << #LHS << " == " << #GEN << " but vectors differ."); return ::rtts::detail::logger{}; } \
        }(LHS, GEN)

#define RTTS_ALL_VALIDATE_BITS(LHS, GEN) \
    [&](auto const& rtts_a, auto const& rtts_gen) { \
        bool rtts_ok = true; \
        size_t rtts_bad = 0; \
        for(size_t rtts_i = 0; rtts_i < std::size(rtts_a); ++rtts_i) { \
            if(::rtts::detail::to_bits(rtts_a[rtts_i]) != ::rtts::detail::to_bits(rtts_gen(rtts_i))) { \
                rtts_ok = false; rtts_bad = rtts_i; break; \
            } \
        } \
        if(rtts_ok) { ::rtts::detail::global_runtime.pass(); return ::rtts::detail::logger{false}; } \
        else { \
            RTTS_FAIL("Expected bitwise: " << #LHS << " == " << #GEN \
                << " but differ at [" << rtts_bad << "]: " \
                << ::rtts::detail::bits_to_string(rtts_a[rtts_bad]) \
                << " != " << ::rtts::detail::bits_to_string(rtts_gen(rtts_bad))); \
            return ::rtts::detail::logger{}; \
        } \
    }(LHS, GEN)

    namespace simd {
        constexpr raze::arch::ISA current_isa() {
            return raze::vx::target_isa();
        }

        template <class Mask>
        Mask make_alternating_mask() {
            Mask m;

            for (size_t i = 0; i < Mask::size(); ++i)
                m[i] = (i % 2) == 0;

            return m;
        }

        template <class Mask>
        Mask make_random_mask() {
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

    inline int main() {
        std::size_t done_tests = 0;
        try {
            for (auto& t : detail::suite()) {
                auto test_count = detail::global_runtime.test_count;
                auto failure_count = detail::global_runtime.failure_count;
                detail::global_runtime.fail_status = false;
                t();
                done_tests++;
                if (test_count == detail::global_runtime.test_count) {
                    detail::global_runtime.invalid();
                    std::cout << "[!] - " << detail::current_test << " : EMPTY TEST CASE\n";
                }
#if defined(RTTS_SHOW_VALID)
                else if (failure_count == detail::global_runtime.failure_count) {
                    std::cout << "[V] - " << detail::current_test << "\n";
                }
#endif
            }
        }
        catch (...) {
            std::cout << "@@ ABORTING DUE TO EXCEPTION @@ - " << (detail::suite().size() - done_tests - 1) << " Tests not run\n";
        }
        return detail::global_runtime.report();
    }
}

#define RTTS_UNIQUE3(ID, LINE) ID##LINE
#define RTTS_UNIQUE2(ID, LINE) RTTS_UNIQUE3(ID, LINE)
#define RTTS_UNIQUE(ID)        RTTS_UNIQUE2(ID, __COUNTER__)