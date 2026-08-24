#pragma once 

#include <raze/vx/Abi.h>
#include <src/raze/vx/storage/Size.h>
#include <src/raze/vx/storage/ElementWise.h>
#include <src/raze/vx/storage/BestMaskChunk.h>
#include <src/raze/math/BitTest.h>
#include <src/raze/math/BitTestAndSet.h>
#include <src/raze/math/BitTestAndReset.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class T, class Abi>
class mask_storage {
    static constexpr std::size_t bytes = Abi::size * sizeof(T);
    static constexpr bool use_native = (bytes == 16 && has_sse2<Abi::isa>) ||
        (bytes == 32 && has_avx<Abi::isa>) || (bytes == 64 && has_avx512f<Abi::isa>);
public:
    using abi_type = Abi;
    using tuple_type = std::conditional_t<use_native, typename best_mask_chunk<
        T, Abi, Abi::size>::type, simd_mask_tuple_type<T, Abi>>;

    mask_storage() noexcept = default;
    mask_storage(const mask_storage&) noexcept = default;
    mask_storage(mask_storage&&) noexcept = default;

    ~mask_storage() = default;

    mask_storage& operator=(const mask_storage&) noexcept = default;
    mask_storage& operator=(mask_storage&&) noexcept = default;

    raze_nodiscard static constexpr bool is_native() noexcept {
        return use_native;
    }

    raze_nodiscard static constexpr auto chunks_count() noexcept {
        if constexpr (is_native()) return 1;
        else return simd_tuple_size<tuple_type>::value;
    }

    raze_nodiscard raze_always_inline const tuple_type& storage() const noexcept {
        return _data;
    }

    raze_nodiscard raze_always_inline tuple_type& storage() noexcept {
        return _data;
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline void __for_each_chunk(_Function_&& __f, _Args_&& ... __args) noexcept {
        if constexpr (use_native) __f(_data, std::forward<_Args_>(__args)...);
        else __for_each_tuple(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline void __for_each_chunk_reverse(_Function_&& __f, _Args_&& ... __args) noexcept {
        if constexpr (use_native) __f(_data, std::forward<_Args_>(__args)...);
        else __for_each_tuple_reverse(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_all_of(_Function_&& __f, _Args_&& ... __args) noexcept {
        if constexpr (use_native) return __f(_data, std::forward<_Args_>(__args)...);
        else return __for_each_tuple_all_of(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_all_of_reverse(_Function_&& __f, _Args_&& ... __args) noexcept {
        if constexpr (use_native) return __f(_data, std::forward<_Args_>(__args)...);
        else return __for_each_tuple_all_of_reverse(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_any_of(_Function_&& __f, _Args_&& ... __args) noexcept {
        if constexpr (use_native) return __f(_data, std::forward<_Args_>(__args)...);
        else return __for_each_tuple_any_of(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_any_of_reverse(_Function_&& __f, _Args_&& ... __args) noexcept {
        if constexpr (use_native) return __f(_data, std::forward<_Args_>(__args)...);
        else return __for_each_tuple_any_of_reverse(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline void __for_each_chunk(_Function_&& __f, _Args_&& ... __args) const noexcept {
        if constexpr (use_native) __f(_data, std::forward<_Args_>(__args)...);
        else __for_each_tuple(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline void __for_each_chunk_reverse(_Function_&& __f, _Args_&& ... __args) const noexcept {
        if constexpr (use_native) __f(_data, std::forward<_Args_>(__args)...);
        else __for_each_tuple_reverse(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_all_of(_Function_&& __f, _Args_&& ... __args) const noexcept {
        if constexpr (use_native) return __f(_data, std::forward<_Args_>(__args)...);
        else return __for_each_tuple_all_of(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_all_of_reverse(_Function_&& __f, _Args_&& ... __args) const noexcept {
        if constexpr (use_native) return __f(_data, std::forward<_Args_>(__args)...);
        else return __for_each_tuple_all_of_reverse(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_any_of(_Function_&& __f, _Args_&& ... __args) const noexcept {
        if constexpr (use_native) return __f(_data, std::forward<_Args_>(__args)...);
        else return __for_each_tuple_any_of(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_any_of_reverse(_Function_&& __f, _Args_&& ... __args) const noexcept {
        if constexpr (use_native) return __f(_data, std::forward<_Args_>(__args)...);
        else return __for_each_tuple_any_of_reverse(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    raze_always_inline void __insert(i32 i, bool v) noexcept {
        using Signed = typename IntegerForSizeof<T>::Signed;

        if constexpr (use_native) {
            if constexpr (intrin_type<typename tuple_type::unwrapped_type>) _Insert<Abi::isa>()(ustorage(_data), __i, Signed(-Signed(v)));
            else if constexpr (std::is_same_v<std::remove_cvref_t<typename tuple_type::unwrapped_type>, bool>) _data = v;
            else v ? math::bit_test_and_set(ustorage(_data), i) : math::bit_test_and_reset(ustorage(_data), i);
        }
        else {
            visit_chunk_by_index(_data, i, [&] <class Chunk> (Chunk & chunk, i32 lane) raze_always_inline_lambda {
                if constexpr (intrin_type<typename Chunk::unwrapped_type>) _Insert<Abi::isa>()(ustorage(chunk), lane, Signed(-Signed(v)));
                else if constexpr (std::is_same_v<std::remove_cvref_t<typename Chunk::unwrapped_type>, bool>) chunk = v;
                else v ? math::bit_test_and_set(ustorage(chunk), lane) : math::bit_test_and_reset(ustorage(chunk), lane);
            });
        }
    }

    raze_always_inline bool __extract(i32 i) const noexcept {
        if constexpr (use_native) {
            if constexpr (intrin_type<typename tuple_type::unwrapped_type>) return bool(-_Extract<Abi::isa, T>()(ustorage(_data), i));
            else if constexpr (std::is_same_v<std::remove_cvref_t<typename tuple_type::unwrapped_type>, bool>) return ustorage(_data);
            else return math::bit_test(ustorage(_data), i);
        }
        else {
            bool r{};

            visit_chunk_by_index(_data, i, [&] <class Chunk> (const Chunk& chunk, i32 lane) raze_always_inline_lambda {
                if constexpr (intrin_type<typename Chunk::unwrapped_type>) r = bool(-_Extract<Abi::isa, T>()(ustorage(chunk), lane));
                else if constexpr (std::is_same_v<std::remove_cvref_t<typename Chunk::unwrapped_type>, bool>) r = ustorage(chunk);
                else r = math::bit_test(ustorage(chunk), lane);
            });

            return r;
        }
    }
private:
    raze_no_unique_address tuple_type _data;
};

__RAZE_VX_NAMESPACE_END
