#ifndef _PINK_BOX_HPP
#define _PINK_BOX_HPP

#include "allocator.hpp"
#include "base.hpp"

namespace pink {

template <class _Ty, class _Alloc = DefaultAllocator>
struct Box : private _Alloc {
  using Allocator = _Alloc;

  template <class _Alloc2>
  using ReplaceAllocator = Box<_Ty, _Alloc2>;

  using ValueType = ReplaceDefaultAllocator<_Ty, _Alloc>;

private:
  ValueType* __pointer = nullptr;

  template <class... _Ts>
  void __construct_ptr(_Ts&&... _Vs) noexcept(
    noexcept(::pink::construct<ValueType>(declval<_Ts&&>()...))) {
    __pointer = static_cast<ValueType*>(
      _Alloc::allocate(sizeof(ValueType), alignof(ValueType)));
    if constexpr (uses_default_allocator<_Ty>) {
      constexpr bool __noexcept = noexcept(ValueType(
        AllocatorArgument(), allocator(), declval<_Ts&&>()...));
      if constexpr (__noexcept) {
        new (__pointer) ValueType(
          AllocatorArgument(), allocator(), static_cast<_Ts&&>(_Vs)...);
      } else {
        try {
          new (__pointer) ValueType(
            AllocatorArgument(),
            allocator(),
            static_cast<_Ts&&>(_Vs)...);
        } catch (...) {
          _Alloc::free(
            __pointer, sizeof(ValueType), alignof(ValueType));
          throw;
        }
      }
    } else if constexpr (noexcept(ValueType(declval<_Ts&&>()...))) {
      new (__pointer)
        ValueType(construct<ValueType>(static_cast<_Ts&&>(_Vs)...));
    } else {
      try {
        new (__pointer)
          ValueType(construct<ValueType>(static_cast<_Ts&&>(_Vs)...));
      } catch (...) {
        _Alloc::free(__pointer, sizeof(ValueType), alignof(ValueType));
        throw;
      }
    }
  }

  void __destroy_ptr() noexcept {
    __pointer->~ValueType();
    _Alloc::free(__pointer, sizeof(ValueType), alignof(ValueType));
    __pointer = nullptr;
  }

  template <class _T2, class _A2>
  constexpr bool __equal_alloc(Box<_T2, _A2> const& __other) const {
    if constexpr (equality_comparable<_Alloc, _A2>) {
      return allocator == __other.allocator();
    } else {
      return false;
    }
  }

  template <class _T2, class _A2>
  using _VTOf = typename Box<_T2, _A2>::ValueType;

  template <class _T2, class _A2>
  constexpr Box& __move_assign(Box<_T2, _A2>& __other) noexcept(
    noexcept(declval<ValueType&>() = declval<_VTOf<_T2, _A2>&&>())
    and noexcept(__construct_ptr(declval<_VTOf<_T2, _A2>&&>()))) {
    using _VT2 = _VTOf<_T2, _A2>;

    if (not __pointer) {
      if constexpr (same_type<ValueType, _VT2>) {
        if constexpr (always_equal<_Alloc, _A2>) {
          __pointer = exchange(__other.__pointer, nullptr);
        } else if (__equal_alloc(__other.allocator())) {
          __pointer = exchange(__other.__pointer, nullptr);
        } else if (__other.__pointer) {
          __construct_ptr(static_cast<_VT2&&>(*__other.__pointer));
        }
      } else if (__other.__pointer) {
        __construct_ptr(static_cast<_VT2&&>(*__other.__pointer));
      }
    } else if (not __other.__pointer) {
      __destroy_ptr();
    } else {
      *__pointer = static_cast<_VT2&&>(*__other.__pointer);
    }

    return *this;
  }

  template <class _T2, class _A2>
  constexpr Box& __copy_assign(Box<_T2, _A2> const& __other) noexcept(
    noexcept(declval<ValueType&>() = declval<_VTOf<_T2, _A2> const&>())
    and noexcept(__construct_ptr(declval<_VTOf<_T2, _A2> const&>()))) {
    if (not __pointer and __other.__pointer) {
      __construct_ptr(*__other);
    } else if (not __other.__pointer) {
      __destroy_ptr();
    } else {
      *__pointer = *__other;
    }

    return *this;
  }

public:
  template <class _A>
  constexpr Box(AllocatorArgument, _A&& __alloc) noexcept(
    noexcept(_Alloc(declval<_A&&>())))
    : _Alloc(static_cast<_A&&>(__alloc)) {}

  // clang-format off
  template <class _A, class _T2, class _A2>
  constexpr Box(
    AllocatorArgument, _A&& __alloc, Box<_T2, _A2>&& __other
  ) noexcept(
    noexcept(_Alloc(declval<_A&&>()))
    and (
      noexcept(
        declval<Box&>().__construct_ptr(declval<_VTOf<_T2, _A2>>())
      ) or (
        always_equal<_Alloc, _A2>
        and same_type<ValueType, _VTOf<_T2, _A2>>
      )
    )
  ) : _Alloc(static_cast<_A&&>(__alloc)) {
    // clang-format on
    using _VT2 = _VTOf<_T2, _A2>;

    if constexpr (same_type<ValueType, _VT2>) {
      if constexpr (always_equal<_Alloc, _A2>) {
        __pointer = exchange(__other.__pointer, nullptr);
      } else if (__alloc_equal(__other)) {
        __pointer = exchange(__other.__pointer, nullptr);
      } else if (__other.__pointer) {
        __construct_ptr(static_cast<_VT2&&>(*__other.__pointer));
      }
    } else if (__other.__pointer) {
      __construct_ptr(static_cast<_VT2&&>(*__other.__pointer));
    }
  }

  // clang-format off
  template <class _A, class _T2, class _A2>
  constexpr Box(
    AllocatorArgument,
    _A&& __alloc,
    Box<_T2, _A2> const& __other
  ) noexcept(
    noexcept(_Alloc(declval<_A&&>()))
    and noexcept(declval<Box&>().__construct_ptr(*__other))
  ) : _Alloc(static_cast<_A&&>(__alloc)) {
    // clang-format on
    if (__other.__pointer) {
      __construct_ptr(*__other);
    }
  }

  // clang-format on
  template <class _A, class... _Ts>
  constexpr Box(AllocatorArgument, _A&& __alloc, _Ts&&... __vs) noexcept(
    noexcept(_Alloc(declval<_A&&>()))
    and noexcept(declval<Box&>().__construct_ptr(declval<_Ts&&>()...)))
    : _Alloc(static_cast<_A&&>(__alloc)) {
    // clang-format on
    __construct_ptr(static_cast<_Ts&&>(__vs)...);
  }

  // clang-format off

  constexpr Box()
    _PINK_FORWARDING_CTOR(Box, AllocatorArgument(), _Alloc())

  template <class _T2, class _A2>
  constexpr Box(Box<_T2, _A2>&& __other)
    _PINK_FORWARDING_CTOR(
      Box,
      AllocatorArgument(),
      _Alloc(),
      static_cast<Box<_T2, _A2>&&>(__other)
    )
  constexpr Box(Box&& __other)
    _PINK_FORWARDING_CTOR(
      Box, AllocatorArgument(), _Alloc(), static_cast<Box&&>(__other)
    )

  template <class _T2, class _A2>
  constexpr Box(Box<_T2, _A2> const& __other)
    _PINK_FORWARDING_CTOR(Box, AllocatorArgument(), _Alloc(), __other)
  constexpr Box(Box const& __other)
    _PINK_FORWARDING_CTOR(Box, AllocatorArgument(), _Alloc(), __other)

  // make certain that there are more than 1 arguments for
  // non-explicit
  template <class _T1, class _T2, class... _Ts>
  constexpr Box(_T1&& __v1, _T2&& __v2, _Ts&&... __vs)
    _PINK_FORWARDING_CTOR(
      Box,
      AllocatorArgument(),
      _Alloc(),
      static_cast<_T1&&>(__v1),
      static_cast<_T2&&>(__v2),
      static_cast<_Ts&&>(__vs)...
    )

  template <typename _T1>
  explicit constexpr Box(_T1&& __v)
    _PINK_FORWARDING_CTOR(
      Box, AllocatorArgument(), _Alloc(), static_cast<_T1&&>(__v)
    )

  // assignment operators
  template <class _T2, class _A2>
  constexpr auto operator=(Box<_T2, _A2>&& __other)
    _PINK_FORWARDING_FUNC(__move_assign(__other))

  constexpr auto operator=(Box&& __other)
    _PINK_FORWARDING_FUNC(__move_assign(__other))

  template <class _T2, class _A2>
  constexpr auto operator=(Box<_T2, _A2> const& __other)
    _PINK_FORWARDING_FUNC(__copy_assign(__other))
  constexpr auto operator=(Box const& __other)
    _PINK_FORWARDING_FUNC(__copy_assign(__other))

  // destructor
  ~Box() {
    __destroy_ptr();
  }
  // we turn clang-format back on here, and not before the dtor,
  // to stop clang-format from being "off"

  // clang-format on

  // accessors
  constexpr explicit operator bool() const noexcept {
    return __pointer == nullptr;
  }

  constexpr ValueType& operator*() noexcept {
    return *__pointer;
  }
  constexpr ValueType const& operator*() const noexcept {
    return *__pointer;
  }

  constexpr ValueType* operator->() noexcept {
    return __pointer;
  }
  constexpr ValueType const* operator->() const noexcept {
    return __pointer;
  }

  constexpr ValueType* pointer() noexcept {
    return __pointer;
  }
  constexpr ValueType const* pointer() const noexcept {
    return __pointer;
  }

  constexpr _Alloc const& allocator() const noexcept {
    return (_Alloc const&)*this;
  }

  template <class _T2, class _A2>
  friend constexpr void swap(Box& __lhs, Box<_T2, _A2>& __rhs) noexcept(
      always_equal<_Alloc, _A2> or
      (noexcept(swap(*__lhs.__pointer, *__rhs.__pointer)) and
       noexcept(__lhs.__construct_ptr(declval<ValueType&&>())))) {
    using _VT2 = typename Box<_T2, _A2>::ValueType;

    if constexpr (always_equal<_Alloc, _A2>) {
      swap(__lhs.__pointer, __rhs.__pointer);
    } else if constexpr (equality_comparable<_Alloc, _A2>) {
      if (__lhs.allocator() == __rhs.allocator()) {
        swap(__lhs.__pointer, __rhs.__pointer);
      } else if (__lhs and __rhs) {
        swap(*__lhs.__pointer, *__rhs.__pointer);
      } else if (__lhs) {
        __rhs.__construct_ptr(
          static_cast<ValueType&&>(*__lhs.__pointer));
        __lhs.__destroy_ptr();
      } else if (__rhs) {
        __lhs.__construct_ptr(static_cast<_VT2&&>(*__rhs.__pointer));
        __rhs.__destroy_ptr();
      }
    } else if (__lhs and __rhs) {
      swap(*__lhs.__pointer, *__rhs.__pointer);
    } else if (__lhs) {
      __rhs.__construct_ptr(static_cast<ValueType&&>(*__lhs.__pointer));
      __lhs.__destroy_ptr();
    } else if (__rhs) {
      __lhs.__construct_ptr(static_cast<_VT2&&>(*__rhs.__pointer));
      __rhs.__destroy_ptr();
    }
  }
};

// comparisons
template <class _T1, class _A1, class _T2, class _A2>
constexpr bool operator==(
  Box<_T1, _A1> const& __lhs,
  Box<_T2, _A2> const& __rhs) noexcept(noexcept(*__lhs == *__rhs)) {
  if (bool(__lhs) and bool(__rhs)) {
    return *__lhs == *__rhs;
  } else {
    return bool(__lhs) != bool(__rhs);
  }
}

template <class _T1, class _A1, class _T2, class _A2>
constexpr bool operator!=(
  Box<_T1, _A1> const& __lhs,
  Box<_T2, _A2> const& __rhs) noexcept(noexcept(*__lhs != *__rhs)) {
  if (bool(__lhs) and bool(__rhs)) {
    return *__lhs != *__rhs;
  } else {
    return bool(__lhs) != bool(__rhs);
  }
}

template <class _T1, class _A1, class _T2, class _A2>
constexpr bool operator<(
  Box<_T1, _A2> const& __lhs,
  Box<_T2, _A2> const& __rhs) noexcept(noexcept(*__lhs < *__rhs)) {
  if (bool(__lhs) and bool(__rhs)) {
    return *__lhs < *__rhs;
  } else {
    return bool(__lhs) < bool(__rhs);
  }
}
template <class _T1, class _A1, class _T2, class _A2>
constexpr bool operator>(
  Box<_T1, _A2> const& __lhs,
  Box<_T2, _A2> const& __rhs) noexcept(noexcept(*__lhs > *__rhs)) {
  if (bool(__lhs) and bool(__rhs)) {
    return *__lhs > *__rhs;
  } else {
    return bool(__lhs) > bool(__rhs);
  }
}

template <class _T1, class _A1, class _T2, class _A2>
constexpr bool operator<=(
  Box<_T1, _A2> const& __lhs,
  Box<_T2, _A2> const& __rhs) noexcept(noexcept(*__lhs > *__rhs)) {
  if (bool(__lhs) and bool(__rhs)) {
    return *__lhs <= *__rhs;
  } else {
    return bool(__lhs) <= bool(__rhs);
  }
}
template <class _T1, class _A1, class _T2, class _A2>
constexpr bool operator>=(
  Box<_T1, _A1> const& __lhs,
  Box<_T2, _A2> const& __rhs) noexcept(noexcept(*__lhs > *__rhs)) {
  if (bool(__lhs) and bool(__rhs)) {
    return *__lhs >= *__rhs;
  } else {
    return bool(__lhs) >= bool(__rhs);
  }
}

} // namespace pink

#endif // _PINK_BOX_HPP
