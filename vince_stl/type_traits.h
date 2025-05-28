#ifndef VINCE_TYPE_TRAITS_H__
#define VINCE_TYPE_TRAITS_H__

#include <type_traits>

namespace vince {

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
struct satisfies {
    static constexpr bool value = Pred<Arg>::value && satisfies<Pred, Args...>::value;
};

template <template <class> class Pred, class Arg>
struct satisfies<Pred, Arg> {
    static constexpr bool value = Pred<Arg>::value;
};

template <template <class> class Pred, class... Args>
inline constexpr bool is_satisfied_v = satisfies<Pred, Args...>::value;

}  // namespace vince

#endif