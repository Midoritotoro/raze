#pragma once 

#include <src/raze/vx/hw/Mask.h>
#include <raze/vx/Algorithm.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskBroadcast.h>
#include <src/raze/vx/hw/x86/mask/operations/LoadMask.h>
#include <src/raze/vx/hw/x86/mask/operations/StoreMask.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>
#include <src/raze/vx/Storage.h>
#include <src/raze/vx/reference/SimdMaskReference.h>
#include <bitset>
#include <src/raze/utility/UninitializedTag.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class T, class Abi>
class raze_aligned_type(64) simd_mask {
	static_assert(traits::is_vector_type_supported_v<T>);
public:
	using storage_type = mask_storage<T, Abi>;
	using element_type = T;
	using reference_type = simd_mask_reference<simd_mask>;
	using value_type = T;
	using abi_type = Abi;

	raze_always_inline simd_mask() noexcept {
		_storage.__for_each_chunk([&] <class Chunk> (Chunk& chunk) raze_always_inline_lambda {
			chunk = mask_zero_<abi_type::isa, typename Chunk::unwrapped_type>();
		});
	}

	simd_mask(const simd_mask&) = default;
	simd_mask(simd_mask&&) = default;
	~simd_mask() = default;

	raze_always_inline simd_mask(uninitialized_tag) noexcept {}

	raze_no_stack_protector raze_always_inline explicit simd_mask(bool v) noexcept {
		_storage.__for_each_chunk([&] <class Chunk> (Chunk& chunk) raze_always_inline_lambda {
			chunk = mask_broadcast_<abi_type::isa, Chunk::size, typename Chunk::unwrapped_type, value_type>(v);
		});
	}

	template <std::forward_iterator FwdIt, class Policy = unaligned_policy>
	raze_no_stack_protector raze_always_inline simd_mask(FwdIt it, Policy&& policy = {}) noexcept {
		copy_from(it, policy);
	}

	template <std::forward_iterator FwdIt, class Policy = unaligned_policy>
	raze_no_stack_protector raze_always_inline void copy_from(FwdIt it, Policy&& policy = {}) noexcept
		requires(std::convertible_to<std::iter_value_t<FwdIt>, bool>)
	{
		using Unwrapped = algorithm::unwrapped_iterator_type<FwdIt>;
		using Value = std::iter_value_t<Unwrapped>;

		if constexpr (sizeof(Value) == 1 && std::contiguous_iterator<Unwrapped> && sizeof(T) == 1) {
			auto current = reinterpret_cast<const bool*>(std::to_address(it));

			__for_each_chunk([&] <class Chunk> (Chunk& chunk) raze_always_inline_lambda {
				chunk = load_mask_<abi_type::isa, Chunk::size, typename Chunk::unwrapped_type, value_type>(current, policy);
				algorithm::advance_bytes(current, Chunk::size * sizeof(value_type));
			});
		}
		else {
			for (auto i = 0; i < size(); ++i)
				__insert(i, static_cast<bool>(*it++));
		}
	}

	template <class OutIt, class Policy = unaligned_policy>
	raze_always_inline void copy_to(OutIt out, Policy&& policy = {}) noexcept {
		using Unwrapped = algorithm::unwrapped_iterator_type<OutIt>;
		using Value = std::iter_value_t<Unwrapped>;

		if constexpr (sizeof(Value) == 1 && std::contiguous_iterator<Unwrapped> && sizeof(T) == 1) {
			auto current = reinterpret_cast<bool*>(std::to_address(out));

			__for_each_chunk([&] <class Chunk> (const Chunk& chunk) raze_always_inline_lambda {
				store_mask_<abi_type::isa, Chunk::size, value_type>(current, chunk.data(), policy);
				algorithm::advance_bytes(current, Chunk::size * sizeof(value_type));
			});
		}
		else {
			for (auto i = 0; i < size(); ++i)
				*out++ = __extract(i);
		}
	}

	raze_always_inline bool operator[](i32 i) const noexcept {
		return __extract(i);
	}

	raze_always_inline auto operator[](i32 i) noexcept {
		return reference_type(*this, i);
	}

	raze_nodiscard raze_always_inline static constexpr auto size() noexcept {
		return abi_type::size;
	}

	raze_always_inline simd_mask& operator=(const simd_mask& other) noexcept {
		_storage = other._storage;
		return *this;
	}

	friend raze_always_inline simd_mask operator&(const simd_mask& x, const simd_mask& y) noexcept {
		return bit_and(x, y);
	}

	friend raze_always_inline simd_mask operator|(const simd_mask& x, const simd_mask& y) noexcept {
		return bit_or(x, y);
	}

	friend raze_always_inline simd_mask operator^(const simd_mask& x, const simd_mask& y) noexcept {
		return bit_xor(x, y);
	}

	raze_always_inline simd_mask& operator&=(const simd_mask& other) noexcept {
		return *this = (*this & other);
	}

	raze_always_inline simd_mask& operator|=(const simd_mask& other) noexcept {
		return *this = (*this | other);
	}

	raze_always_inline simd_mask& operator^=(const simd_mask& other) noexcept {
		return *this = (*this ^ other);
	}

	raze_always_inline simd_mask operator!() const noexcept {
		return bit_not(*this);
	}

	friend raze_always_inline simd_mask operator!=(const simd_mask& x, const simd_mask& y) noexcept {
		return bit_xor(x, y);
	}

	friend raze_always_inline simd_mask operator==(const simd_mask& x, const simd_mask& y) noexcept {
		return !(x != y);
	}

	raze_nodiscard raze_always_inline const storage_type& __storage() const noexcept {
		return _storage;
	}

	raze_nodiscard raze_always_inline storage_type& __storage() noexcept {
		return _storage;
	}

	raze_nodiscard raze_always_inline static constexpr bool is_native() noexcept {
		return storage_type::is_native();
	}

	raze_nodiscard raze_always_inline static constexpr auto __chunks_count() noexcept {
		return storage_type::chunks_count();
	}

	template <sizetype I>
	raze_always_inline auto __get() const noexcept {
		return raze::vx::get<I>(_storage.storage());
	}

	template <class F, class ... Args>
	raze_always_inline void __for_each_chunk(F&& f, Args&& ... args) noexcept {
		_storage.__for_each_chunk(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline void __for_each_chunk_reverse(F&& f, Args&& ... args) noexcept {
		_storage.__for_each_chunk_reverse(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline void __for_each_chunk(F&& f, Args&& ... args) const noexcept {
		_storage.__for_each_chunk(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline void __for_each_chunk_reverse(F&& f, Args&& ... args) const noexcept {
		_storage.__for_each_chunk_reverse(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline bool __for_each_chunk_any_of(F&& f, Args&& ... args) noexcept {
		return _storage.__for_each_chunk_any_of(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline bool __for_each_chunk_any_of_reverse(F&& f, Args&& ... args) noexcept {
		return _storage.__for_each_chunk_any_of_reverse(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline bool __for_each_chunk_any_of(F&& f, Args&& ... args) const noexcept {
		return _storage.__for_each_chunk_any_of(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline bool __for_each_chunk_any_of_reverse(F&& f, Args&& ... args) const noexcept {
		return _storage.__for_each_chunk_any_of_reverse(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline bool __for_each_chunk_all_of(F&& f, Args&& ... args) noexcept {
		return _storage.__for_each_chunk_all_of(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline bool __for_each_chunk_all_of_reverse(F&& f, Args&& ... args) noexcept {
		return _storage.__for_each_chunk_all_of_reverse(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline bool __for_each_chunk_all_of(F&& f, Args&& ... args) const noexcept {
		return _storage.__for_each_chunk_all_of(std::forward<F>(f), std::forward<Args>(args)...);
	}

	template <class F, class ... Args>
	raze_always_inline bool __for_each_chunk_all_of_reverse(F&& f, Args&& ... args) const noexcept {
		return _storage.__for_each_chunk_all_of_reverse(std::forward<F>(f), std::forward<Args>(args)...);
	}

	raze_always_inline void __insert(i32 i, bool v) noexcept {
		_storage.insert(i, v);
    }

    raze_nodiscard raze_always_inline bool __extract(i32 i) const noexcept {
		return _storage.extract(i);
    }
private:
	raze_no_unique_address storage_type _storage;
};

__RAZE_VX_NAMESPACE_END