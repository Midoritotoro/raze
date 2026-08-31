#pragma once 

#include <raze/vx/Abi.h>
#include <src/raze/vx/storage/Size.h>
#include <src/raze/vx/storage/ElementWise.h>
#include <src/raze/vx/storage/BestChunk.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class T, class Abi>
class vector_storage {
    static constexpr std::size_t bytes = Abi::size * sizeof(T);
    static constexpr bool use_native = (bytes == 16 && has_sse2<Abi::isa>) ||
        (bytes == 32 && has_avx<Abi::isa>) || (bytes == 64 && has_avx512f<Abi::isa>);
public:
    using abi_type = Abi;
    using tuple_type = std::conditional_t<use_native, typename best_chunk<
        T, Abi, Abi::size>::type, simd_vector_tuple_type<T, Abi>>;
    
    raze_always_inline vector_storage() noexcept = default;
    raze_always_inline vector_storage(const vector_storage&) noexcept = default;
    raze_always_inline vector_storage(vector_storage&&) noexcept = default;

    raze_always_inline ~vector_storage() = default;

    raze_always_inline vector_storage& operator=(const vector_storage&) noexcept = default;
    raze_always_inline vector_storage& operator=(vector_storage&&) noexcept = default;

    raze_nodiscard static constexpr bool is_native() noexcept {
        return use_native;
    }

    raze_nodiscard static constexpr auto chunks_count() noexcept {
        if constexpr (is_native()) return 1;
        else return simd_tuple_size<std::remove_cvref_t<tuple_type>>::value;
    }

    raze_always_inline tuple_type storage() const noexcept {
        return _data;
    }

    raze_always_inline tuple_type& storage() noexcept {
        return _data;
    }

    template <class F, class ... Args>
    raze_always_inline void __for_each_chunk(F&& f, Args&& ... args) noexcept {
        if constexpr (use_native) f(_data, std::forward<Args>(args)...);
        else for_each_tuple(_data, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <class F, class ... Args>
    raze_always_inline void __for_each_chunk_reverse(F&& f, Args&& ... args) noexcept {
        if constexpr (use_native) f(_data, std::forward<Args>(args)...);
        else for_each_tuple_reverse(_data, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <class F, class ... Args>
    raze_always_inline auto __for_each_chunk_all_of(F&& f, Args&& ... args) noexcept {
        if constexpr (use_native) return f(_data, std::forward<Args>(args)...);
        else return for_each_tuple_all_of(_data, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <class F, class ... Args>
    raze_always_inline auto __for_each_chunk_all_of_reverse(F&& f, Args&& ... args) noexcept {
        if constexpr (use_native) return f(_data, std::forward<Args>(args)...);
        else return for_each_tuple_all_of_reverse(_data, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <class F, class ... Args>
    raze_always_inline auto __for_each_chunk_any_of(F&& f, Args&& ... args) noexcept {
        if constexpr (use_native) return f(_data, std::forward<Args>(args)...);
        else return for_each_tuple_any_of(_data, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <class F, class ... Args>
    raze_always_inline auto __for_each_chunk_any_of_reverse(F&& f, Args&& ... args) noexcept {
        if constexpr (use_native) return f(_data, std::forward<Args>(args)...);
        else return for_each_tuple_any_of_reverse(_data, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <class F, class ... Args>
    raze_always_inline void __for_each_chunk(F&& f, Args&& ... args) const noexcept {
        if constexpr (use_native) f(_data, std::forward<Args>(args)...);
        else for_each_tuple(_data, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <class F, class ... Args>
    raze_always_inline void for_each_chunk_reverse(F&& f, Args&& ... args) const noexcept {
        if constexpr (use_native) f(_data, std::forward<Args>(args)...);
        else for_each_tuple_reverse(_data, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <class F, class ... Args>
    raze_always_inline auto __for_each_chunk_all_of(F&& f, Args&& ... args) const noexcept {
        if constexpr (use_native) return f(_data, std::forward<Args>(args)...);
        else return for_each_tuple_all_of(_data, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <class F, class ... Args>
    raze_always_inline auto __for_each_chunk_all_of_reverse(F&& f, Args&& ... args) const noexcept {
        if constexpr (use_native) return f(_data, std::forward<Args>(args)...);
        else return for_each_tuple_all_of_reverse(_data, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <class F, class ... Args>
    raze_always_inline auto __for_each_chunk_any_of(F&& f, Args&& ... args) const noexcept {
        if constexpr (use_native) return f(_data, std::forward<Args>(args)...);
        else return for_each_tuple_any_of(_data, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <class F, class ... Args>
    raze_always_inline auto __for_each_chunk_any_of_reverse(F&& f, Args&& ... args) const noexcept {
        if constexpr (use_native) return f(_data, std::forward<Args>(args)...);
        else return for_each_tuple_any_of_reverse(_data, std::forward<F>(f), std::forward<Args>(args)...);
    }

    raze_always_inline void insert(i32 i, T v) noexcept {
        raze_debug_assert(i >= 0 && i < Abi::size);

        if constexpr (use_native) insert_<Abi::isa>(ustorage(_data), i, v);
        else visit_chunk_by_index(_data, i, [&](auto& chunk, i32 lane) raze_always_inline_lambda {
            insert_<Abi::isa>(ustorage(chunk), lane, v);
        });
    }

    raze_nodiscard raze_always_inline T extract(i32 i) const noexcept {
        raze_debug_assert(i >= 0 && i < Abi::size);
        T r{};

        if constexpr (use_native) r = extract_<Abi::isa, T>(ustorage(_data), i);
        else visit_chunk_by_index(_data, i, [&](const auto& chunk, i32 lane) raze_always_inline_lambda {
            r = extract_<Abi::isa, T>(ustorage(chunk), lane);
        });

        return r;
    }

    template <sizetype I>
    raze_nodiscard raze_always_inline T extract(std::integral_constant<sizetype, I> i) const noexcept {
        static_assert(i >= 0 && i < Abi::size);
        T r{};

        if constexpr (use_native) r = extract_<Abi::isa, T>(ustorage(_data), i);
        else visit_chunk_by_index(_data, i, [&](const auto& chunk, auto current) raze_always_inline_lambda{
            r = extract_<Abi::isa, T>(ustorage(chunk), current);
        });

        return r;
    }
private:
    raze_no_unique_address tuple_type _data;
};

__RAZE_VX_NAMESPACE_END
