#ifndef _PINK_TYPE_TRAITS_HPP
#define _PINK_TYPE_TRAITS_HPP

namespace pink {

template <class _Ty>
_Ty declval() noexcept;

template <class _Ty, _Ty _Val>
struct Constant {
  constexpr static _Ty value = _Val;

  constexpr operator _Ty() const {
    return _Val;
  }
  constexpr _Ty operator()() const {
    return _Val;
  }
};

template <bool _Val>
using BoolConstant = Constant<bool, _Val>;

using TrueType = BoolConstant<true>;
using FalseType = BoolConstant<false>;

namespace __impl {
  template <class _Ty, class... _Ts>
  struct SfinaeAs {
    using Type = _Ty;
  };

  template <bool __v>
  struct EnableIf {};

  template <>
  struct EnableIf<true> {
    template <class _Ty>
    using Type = _Ty;
  };
} // namespace __impl

template <class _Ty, class... _Ts>
using SfinaeAs = typename __impl::SfinaeAs<_Ty, _Ts...>::Type;

template <class... _Ts>
using Sfinae = SfinaeAs<void, _Ts...>;

template <bool __v, class _Ty = void>
using EnableIf = typename __impl::EnableIf<__v>::template Type<_Ty>;

namespace __impl {
  template <class _Ty, class _Void = void>
  struct DefaultConstructible : FalseType {};

  template <class _Ty>
  struct DefaultConstructible<_Ty, Sfinae<decltype(_Ty())>>
    : TrueType {};

  template <class _T1, class _T2, class _Void = void>
  struct EqualityComparable : FalseType {};

  template <class _T1, class _T2>
  struct EqualityComparable<
    _T1,
    _T2,
    Sfinae<
      decltype(declval<_T1>() == declval<_T2>()),
      decltype(declval<_T1>() != declval<_T2>())>> : TrueType {};

  template <class _T1, class _T2>
  struct SameType : FalseType {};

  template <class _T1>
  struct SameType<_T1, _T1> : TrueType {};
} // namespace __impl

template <class _Ty>
constexpr static bool default_constructible =
  __impl::DefaultConstructible<_Ty>::value;

template <class _T1, class _T2>
constexpr static bool equality_comparable =
  __impl::EqualityComparable<_T1, _T2>::value;

template <class _T1, class _T2>
constexpr static bool same_type = __impl::SameType<_T1, _T2>::value;

template <class _T1, class _T2 = _T1>
struct TraitAlwaysEqual : FalseType {};

template <class _T1, class _T2 = _T1>
constexpr static bool always_equal = TraitAlwaysEqual<_T1, _T2>::value;

} // namespace pink

#endif // _PINK_TYPE_TRAITS_HPP
