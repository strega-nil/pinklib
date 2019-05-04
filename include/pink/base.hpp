#ifndef _PINK_BASE_HPP
#define _PINK_BASE_HPP

#include "type_traits.hpp"

namespace pink {

using Int = decltype("" - "");

namespace __impl {
  template <class _T1>
  constexpr void swap(_T1& _Lhs, _T1& _Rhs) noexcept {
    _T1 _Tmp = static_cast<_T1&&>(_Lhs);
    _Lhs = static_cast<_T1&&>(_Rhs);
    _Rhs = static_cast<_T1&&>(_Tmp);
  }
} // namespace __impl

template <class _T1>
constexpr void swap(_T1& _Lhs, _T1& _Rhs) noexcept {
  using __impl::swap;
  swap(_Lhs, _Rhs);
}

template <class _T1, class _T2>
constexpr _T1 exchange(_T1& __lhs, _T2&& __rhs) noexcept(
  noexcept(_T1(declval<_T1&&>()))
  and noexcept(__lhs = declval<_T2&&>())) {
  _T1 tmp = static_cast<_T1&&>(__lhs);
  __lhs = static_cast<_T2&&>(__rhs);
  return tmp;
}

template <class _Ty>
constexpr _Ty construct() noexcept(noexcept(_Ty())) {
  return _Ty();
}

template <class _Ty, class _T1>
constexpr _Ty construct(_T1&& _V1) noexcept(
  noexcept(static_cast<_Ty>(declval<_T1&&>()))) {
  return static_cast<_Ty>(static_cast<_T1&&>(_V1));
}

template <class _Ty, class... _Ts>
constexpr _Ty
construct(_Ts&&... __vs) noexcept(noexcept(_Ty(declval<_Ts&&>()...))) {
  return _Ty(static_cast<_Ts&&>(__vs)...);
}

[[noreturn]] void _Fail(
  char const* _Msg, char const* _Func, int _Line, char const* _File);

} // namespace pink

#define _PINK_FAIL(s) ::pink::_Fail(s, __func__, __LINE__, __FILE__)

#define _PINK_EXPECT(b) \
  do { \
    if (not(b)) { \
      _PINK_FAIL("expect failed: " #b); \
    } \
  } while (false)

#ifdef _DEBUG
#define _PINK_DEBUG_FAIL(s) \
  ::pink::_Fail(s, __func__, __LINE__, __FILE__)
#else
#define _PINK_DEBUG_FAIL(s)
#endif

#define _PINK_DEBUG_EXPECT(b) \
  do { \
    if (not(b)) { \
      _PINK_DEBUG_FAIL("debug expect failed: " #b); \
    } \
  } while (false)

#define _PINK_FORWARDING_FUNC(...) \
  noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__) { \
    return __VA_ARGS__; \
  }

#define _PINK_FORWARDING_CTOR(cls, ...) \
  noexcept(noexcept(cls(__VA_ARGS__))) : cls(__VA_ARGS__) {}

#endif // _PINK_BASE_HPP
