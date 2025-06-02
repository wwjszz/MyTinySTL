#ifndef WYNE_TYPE_TRAITS_H__
#define WYNE_TYPE_TRAITS_H__

#include <type_traits>

namespace wyne {

template <class T, T v>
struct _integral_constant {
    static constexpr T value = v;
};

template <bool b>
using _bool_constant = _integral_constant<bool, b>;

using _true_type  = _bool_constant<true>;
using _false_type = _bool_constant<false>;

// is_pair

template <class T1, class T2>
struct pair;

template <class... T>
struct is_pair : _false_type {};

template <class T1, class T2>
struct is_pair<T1, T2> : _true_type {};

/// is_satisfies

template <template <class> class Pred, class Arg, class... Args>
struct satisfies_and {
    static constexpr bool value = Pred<Arg>::value && satisfies_and<Pred, Args...>::value;
};

template <template <class> class Pred, class Arg>
struct satisfies_and<Pred, Arg> {
    static constexpr bool value = Pred<Arg>::value;
};

template <template <class> class Pred, class Arg, class... Args>
struct satisfies_or {
    static constexpr bool value = Pred<Arg>::value && satisfies_or<Pred, Args...>::value;
};

template <template <class> class Pred, class Arg>
struct satisfies_or<Pred, Arg> {
    static constexpr bool value = Pred<Arg>::value;
};

template <template <class> class Pred, class... Args>
inline constexpr bool is_satisfied_and_v = satisfies_and<Pred, Args...>::value;

template <template <class> class Pred, class... Args>
inline constexpr bool is_satisfied_or_v = satisfies_or<Pred, Args...>::value;

template <class Dp, class Bp>
inline constexpr bool is_derived_from_v =
    std::is_base_of_v<Bp, Dp> && std::is_convertible_v<const volatile Dp*, const volatile Bp*>;

}  // namespace wyne

#endif