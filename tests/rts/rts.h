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
#include <tuple>

namespace rtts {
    template<typename... Ts>
    struct types {
        template<typename... Us> constexpr types<Ts..., Us...> operator+(types<Us...> const&) const;
    };

    template<typename... Ls> struct concatenate { using type = decltype((Ls{} + ...)); };

    template<typename... Ls> using concatenate_t = typename concatenate<Ls...>::type;

    template<typename T> struct type {};

    namespace detail {
        template<typename T> struct typename_impl {
            static auto value() noexcept {
#if defined(_MSC_VER)
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

    template<typename T> inline auto const typename_ = detail::typename_impl<T>::value();

    namespace detail {
        struct env {
            void pass() { test_count++; success_count++; }
            void fail() { test_count++; failure_count++; }
            void invalid() { test_count++; invalid_count++; }

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
            template<typename Data> logger& operator<<(Data const& d) {
                if (display) {
                    if (!done) { std::cout << ">> Additional information: \n"; done = true; }
                    std::cout << d;
                }
                return *this;
            }
            ~logger() noexcept { if (display && done) std::cout << "\n"; }
            bool display, done;
        };

        struct callable {
            using signature_t = void(*)(void*);
            signature_t invoker = {};
            signature_t cleanup = {};
            void* payload = {};

            template<typename Function>
            constexpr callable(Function f)
                : invoker{ invoke<Function> }, cleanup{ destroy<Function> }, payload{ new Function{std::move(f)} } {}

            void operator()() { invoker(payload); }
            template <typename T> static void invoke(void* data) { (*static_cast<T*>(data))(); }
            template <typename T> static void destroy(void* data) { delete static_cast<T*>(data); }
        };

        struct test {
            std::string name;
            callable behaviour;
            void operator()() { current_test = name; behaviour(); }
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

   

        template<typename... Types>
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

        template<typename... Types>
        struct test_captures<types<Types...>> : test_captures<Types...> {};
    }

    template<typename T>
    std::string as_string(T const& e) {
        if constexpr (requires { std::to_string(e); }) {
            return std::to_string(e);
        }
        else if constexpr (requires(std::ostream & o) { o << e; }) {
            std::ostringstream os; os << e; return os.str();
        }
        else {
            return std::string("[") + typename_<T> +"]";
        }
    }
    inline std::string as_string(bool b) { return b ? "true" : "false"; }

    template<typename Simd>
        requires requires { Simd::size(); typename Simd::value_type; }
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

    namespace simd {
        constexpr raze::arch::ISA current_isa() {
            return raze::vx::target_isa();
        }

        template<typename T, raze::u32 Width>
        struct simd_info {
            using base_type = T;
            static constexpr raze::u32 width = Width;
            using type = raze::vx::simd<base_type, raze::vx::runtime_abi<current_isa(), width>>;
        };

        template<typename T>
        struct widths_for_type {
            using type = types<
                simd_info<T, 128>,
                simd_info<T, 256>,
                simd_info<T, 512>
            >;
        };

        template<typename... Lists>
        struct concatenate_lists;

        template<>
        struct concatenate_lists<> {
            using type = types<>;
        };

        template<typename... Ts, typename... Rest>
        struct concatenate_lists<types<Ts...>, Rest...> {
            using type = concatenate_t<
                types<Ts...>,
                typename concatenate_lists<Rest...>::type
            >;
        };

        template <typename... Ts>
        auto make_all_widths(types<Ts...>) {
            return typename concatenate_lists<
                typename widths_for_type<Ts>::type...
            >::type{};
        }

        using base_types = types<raze::i8, raze::u8, raze::i16, raze::u16, raze::i32, raze::u32, raze::i64, raze::u64, raze::f32, raze::f64>;
        using fp_types = types<raze::f32, raze::f64>;

        using all_simd_infos = decltype(make_all_widths(base_types{}));
        using all_fp_infos = decltype(make_all_widths(fp_types{}));

        template<typename Info>
        struct is_valid_simd_info {
            static constexpr bool value = requires {
                typename raze::vx::simd<typename Info::base_type, raze::vx::runtime_abi<current_isa(), Info::width>>;
            };
        };

        template<template<typename> typename Pred, typename Type> struct filter {
            template<typename... Ls> static types<Ls...> tuple_to_types(const std::tuple<Ls...>&);
            template<typename T>
            static std::conditional_t<Pred<T>::value, std::tuple<T>, std::tuple<>> filter_type();
            template<typename... Ls> static auto filter_impl(types<Ls...>) {
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
                else if (failure_count == detail::global_runtime.failure_count) {
                    std::cout << "[V] - " << detail::current_test << "\n";
                }
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