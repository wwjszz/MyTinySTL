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

}  // namespace vince

#endif