#ifndef WYNE_TYPE_TRAITS_H__
#define WYNE_TYPE_TRAITS_H__

#include <type_traits>

namespace wyne {

template <class T, T v>
struct _integral_constant {
    static constexpr T value = v;
};

template <class T, T v>
using integral_constant = _integral_constant<T, v>;

template <std::size_t N>
using size_constant = integral_constant<std::size_t, N>;

template <bool b>
using _bool_constant = _integral_constant<bool, b>;

template <bool b>
using bool_constant = _bool_constant<b>;

using _true_type  = _bool_constant<true>;
using _false_type = _bool_constant<false>;

using true_type  = _true_type;
using false_type = _false_type;

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

// conditional

template <bool Bp, class If, class Then>
struct conditional {
    using type = If;
};

template <class If, class Then>
struct conditional<false, If, Then> {
    using type = Then;
};

template <bool Bp, class If, class Then>
using conditional_t = typename conditional<Bp, If, Then>::type;

// Not

template <class Pred>
struct Not : _bool_constant<!Pred::value> {};

// first_true

template <class...>
struct first_true : public _false_type {};

template <class B1, class... Bn>
struct first_true<B1, Bn...>
    : conditional_t<static_cast<bool>( B1::value ), B1, first_true<Bn...>> {};

// first_false

template <class...>
struct first_false : public _true_type {};

template <class B1, class... Bn>
struct first_false<B1, Bn...>
    : conditional<static_cast<bool>( B1::value ), first_false<Bn...>, B1> {};

// add type

template <class T>
struct add_const {
    using type = const T;
};

template <class T>
struct add_volatile {
    using type = volatile T;
};

template <class T>
struct add_cv {
    using type = const volatile T;
};

template <class T>
using add_const_t = typename add_const<T>::type;

template <class T>
using add_volatile_t = typename add_volatile<T>::type;

template <class T>
using add_cv_t = typename add_cv<T>::type;

}  // namespace wyne

#endif