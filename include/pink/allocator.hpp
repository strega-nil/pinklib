#ifndef _PINK_ALLOCATOR_HPP
#define _PINK_ALLOCATOR_HPP

#include "base.hpp"
#include "type_traits.hpp"

namespace pink {

struct MemoryResource {
  void* allocate(Int __size, Int __align) const noexcept {
    return do_allocate(__size, __align);
  }
  void free(void* __pointer, Int __size, Int __align) const noexcept {
    do_free(__pointer, __size, __align);
  }

  MemoryResource(MemoryResource&&) = delete;
  MemoryResource(MemoryResource const&) = delete;
  MemoryResource& operator=(MemoryResource&&) = delete;
  MemoryResource& operator=(MemoryResource const&) = delete;

protected:
  virtual void* do_allocate(Int __size, Int __align) const noexcept = 0;
  virtual void do_free(void* __pointer, Int __size, Int __align) const
    noexcept = 0;

  MemoryResource() = default;
  ~MemoryResource() = default;
};

struct SystemMemoryResource final : MemoryResource {
  void* do_allocate(Int __size, Int __align) const noexcept override;
  void do_free(void* __pointer, Int __size, Int __align) const
    noexcept override;
};

extern SystemMemoryResource const system_memory_resource;

struct PolymorphicAllocator {
  void* allocate(Int __size, Int __align) const noexcept {
    return __memory->allocate(__size, __align);
  }
  void free(void* __pointer, Int __size, Int __align) const noexcept {
    return __memory->free(__pointer, __size, __align);
  }

  constexpr PolymorphicAllocator() noexcept
    : __memory(&system_memory_resource) {}

  constexpr PolymorphicAllocator(
    MemoryResource const* __memory) noexcept
    : __memory(__memory) {}

  constexpr MemoryResource const* memory_resource() const noexcept {
    return __memory;
  }

private:
  MemoryResource const* __memory;
};

template <auto const* __memory>
struct MonomorphicAllocator {
  void* allocate(Int __size, Int __align) const noexcept {
    return __memory->allocate(__size, __align);
  }
  void free(void* __pointer, Int __size, Int __align) const noexcept {
    __memory->free(__pointer, __size, __align);
  }

  constexpr MemoryResource const* memory_resource() const noexcept {
    return __memory;
  }
};

using SystemAllocator = MonomorphicAllocator<&system_memory_resource>;

struct DefaultAllocator : SystemAllocator {};

constexpr bool operator==(
  PolymorphicAllocator __lhs, PolymorphicAllocator __rhs) noexcept {
  return __lhs.memory_resource() == __rhs.memory_resource();
}
constexpr bool operator!=(
  PolymorphicAllocator __lhs, PolymorphicAllocator __rhs) noexcept {
  return not(__lhs == __rhs);
}

template <auto const* __memory>
constexpr bool operator==(
  PolymorphicAllocator __lhs, MonomorphicAllocator<__memory>) noexcept {
  return __lhs.memory_resource() == __memory;
}
template <auto const* __memory>
constexpr bool operator!=(
  PolymorphicAllocator __lhs, MonomorphicAllocator<__memory>) noexcept {
  return __lhs.memory_resource() != __memory;
}
template <auto const* __memory>
constexpr bool operator==(
  MonomorphicAllocator<__memory>, PolymorphicAllocator __rhs) noexcept {
  return __memory == __rhs.memory_resource();
}
template <auto const* __memory>
constexpr bool operator!=(
  MonomorphicAllocator<__memory>, PolymorphicAllocator __rhs) noexcept {
  return __memory != __rhs.memory_resource();
}
template <auto const* __m1, auto const* __m2>
constexpr bool operator==(
  MonomorphicAllocator<__m1>, MonomorphicAllocator<__m2>) noexcept {
  return static_cast<MemoryResource const*>(__m1)
    == static_cast<MemoryResource const*>(__m2);
}
template <auto const* __m1, auto const* __m2>
constexpr bool operator!=(
  MonomorphicAllocator<__m1>, MonomorphicAllocator<__m2>) noexcept {
  return static_cast<MemoryResource const*>(__m1)
    != static_cast<MemoryResource const*>(__m2);
}

template <auto const* __m1, auto const* __m2>
struct TraitAlwaysEqual<
  MonomorphicAllocator<__m1>,
  MonomorphicAllocator<__m2>> : BoolConstant<__m1 == __m2> {};

template <>
struct TraitAlwaysEqual<DefaultAllocator> : TrueType {};
template <>
struct TraitAlwaysEqual<DefaultAllocator, SystemAllocator>
  : TrueType {};
template <>
struct TraitAlwaysEqual<SystemAllocator, DefaultAllocator>
  : TrueType {};

namespace __impl {

  template <class _Ty, class _Void = void>
  struct AllocatorAware : FalseType {};

  template <class _Ty>
  struct AllocatorAware<
    _Ty,
    Sfinae<
      typename _Ty::template ReplaceAllocator<DefaultAllocator>,
      typename _Ty::Allocator>> : TrueType {};

  template <class _Ty, bool __alloc_aware = AllocatorAware<_Ty>::value>
  struct AllocatorOf {
    using Type = void;
  };

  template <class _Ty>
  struct AllocatorOf<_Ty, true> {
    using Type = typename _Ty::Allocator;
  };

  template <
    class _Ty,
    class _OldAlloc = typename AllocatorOf<_Ty>::Type>
  struct UsesDefaultAllocator : FalseType {};

  template <class _Ty>
  struct UsesDefaultAllocator<_Ty, DefaultAllocator> : TrueType {};

  template <
    class _Ty,
    bool __uses_default_alloc = UsesDefaultAllocator<_Ty>::value>
  struct ReplaceDefaultAllocator {
    template <class>
    using Type = _Ty;
  };

  template <class _Ty>
  struct ReplaceDefaultAllocator<_Ty, true> {
    template <class _NewAlloc>
    using Type = typename _Ty::template ReplaceAllocator<_NewAlloc>;
  };

} // namespace __impl

template <class _Ty>
constexpr static bool allocator_aware =
  __impl::AllocatorAware<_Ty>::value;

template <class _Ty>
constexpr static bool uses_default_allocator =
  __impl::UsesDefaultAllocator<_Ty>::value;

template <class _Ty>
using AllocatorOf = typename __impl::AllocatorOf<_Ty>::Type;

template <class _Ty, class _NewAlloc>
using ReplaceDefaultAllocator =
  typename __impl::ReplaceDefaultAllocator<_Ty>::template Type<
    _NewAlloc>;

struct AllocatorArgument {};

} // namespace pink

#endif // _PINK_ALLOCATOR_HPP
