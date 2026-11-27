#pragma once

#include <src/raze/options/Traits.h>
#include <src/raze/vx/hw/x86/memory/ConditionalMemAccessNative.h>
#include <src/raze/options/As.h>
#include <src/raze/vx/hw/configurable/mask/FirstN.h>
#include <src/raze/algorithm/TailMask.h>
#include <bit>
#include <utility>

__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class Traits>
struct unroller_t {
	template <class Tag>
	struct impl {
		constexpr impl() noexcept = default;
		constexpr ~impl() noexcept = default;

		struct mask_generator {
			typename Tag::mask_type m;

			raze_always_inline explicit mask_generator(i32 n) noexcept
				: m(raze::vx::first_n(n, as<typename Tag::mask_type>()))
			{}

			raze_always_inline typename Tag::mask_type operator()() const noexcept {
				return m;
			}
		};

		static raze_always_inline mask_generator make_mask_generator(i32 n) noexcept {
			return mask_generator(n);
		}

		template <class F>
		static consteval bool can_process_tail() noexcept {
			return requires(F& fn, sizetype n) {
				fn(Tag{ uninitialized }, algorithm::tail_mask(n, std::declval<mask_generator>()));
			};
		}

		template <class F>
		constexpr raze_always_inline auto operator()(sizetype aligned_size, sizetype tail_size, F& f) const
			requires(!std::same_as<Tag, vx::scalar_tag>)
		{
			if constexpr (requires { f(Tag{ uninitialized }, aligned_size); }) {
				constexpr auto has_early_exit = std::same_as<decltype(f(Tag{ uninitialized }, aligned_size)), bool>;

#if defined(raze_cpp_msvc_only) || (defined(raze_cpp_gnu) && !defined(RAZE_GCC_SRA_LARGE_LIMIT))
				// Ignore unrolling
				if constexpr (has_early_exit) {
					if (!f(Tag{ uninitialized }, aligned_size)) {
						if constexpr (requires { f.result(); }) return f.result();
						else return;
					}
				}
				else {
					f(Tag{ uninitialized }, aligned_size);
				}
#else
				constexpr auto unrolling = get_unrolling<Traits>();
				using UnrolledTag = vx::simd<typename Tag::value_type, vx::resize_abi_t<vx::abi_t<Tag>, Tag::size() * unrolling>>;

				if constexpr (unrolling > 1 && requires { f(UnrolledTag{ uninitialized }, aligned_size); }) {
					auto unrolled_size = aligned_size - (aligned_size % (sizeof(Tag) * unrolling));

					if (unrolled_size != 0) {
						if constexpr (has_early_exit) {
							if (!f(UnrolledTag{ uninitialized }, unrolled_size))
							{
								if constexpr (requires { f.result(); }) return f.result();
								else return;
							}
						}
						else {
							f(UnrolledTag{ uninitialized }, unrolled_size);
						}
					}

					aligned_size -= unrolled_size;

					if (aligned_size >= sizeof(Tag)) {
						if constexpr (has_early_exit) {
							if (!f(Tag{ uninitialized }, aligned_size)) {
								if constexpr (requires { f.result(); }) return f.result();
								else return;
							}
						}
						else {
							f(Tag{ uninitialized }, aligned_size);
						}
					}
				}
				else {
					if constexpr (has_early_exit) {
						if (!f(Tag{ uninitialized }, aligned_size)) {
							if constexpr (requires { f.result(); }) return f.result();
							else return;
						}
					}
					else {
						f(Tag{ uninitialized }, aligned_size);
					}
				}
#endif // defined(raze_cpp_msvc_only) || (defined(raze_cpp_gnu) && !defined(RAZE_GCC_SRA_LARGE_LIMIT))

				constexpr auto shift = std::countr_zero(sizeof(typename Tag::value_type));

				if constexpr (vx::native_conditional_memory_access<vx::abi_t<Tag>::isa, typename Tag::value_type> && can_process_tail<F>()) {
					f(Tag{ uninitialized }, algorithm::tail_mask(tail_size, make_mask_generator(static_cast<i32>(tail_size >> shift))));
				}
				else {
					f();
				}
			}
			else {
				f();
			}

			if constexpr (requires { f.result(); }) return f.result();
		}

		template <class F>
		constexpr raze_always_inline auto operator()(sizetype tail_size, F& f) const
			requires(!std::same_as<Tag, vx::scalar_tag>)
		{
			constexpr auto shift = std::countr_zero(sizeof(typename Tag::value_type));

			if constexpr (vx::native_conditional_memory_access<vx::abi_t<Tag>::isa, typename Tag::value_type> && can_process_tail<F>()) {
				f(Tag{ uninitialized }, algorithm::tail_mask(tail_size, make_mask_generator(static_cast<i32>(tail_size >> shift))));
			}
			else {
				f();
			}

			if constexpr (requires { f.result(); }) return f.result();
		}

		template <class F>
		constexpr raze_always_inline auto operator()(F& f) const
			requires(std::same_as<Tag, vx::scalar_tag>)
		{
			f();
			if constexpr (requires { f.result(); }) return f.result();
		}
	};
};

template <class Traits, class V>
static inline constexpr auto unroller = typename unroller_t<Traits>::template impl<V>{};

__RAZE_OPTIONS_NAMESPACE_END