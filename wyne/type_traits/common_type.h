#ifndef WYNE_COMMON_TYPE_H__
#define WYNE_COMMON_TYPE_H__

#include <type_traits>
#include <utility>

namespace wyne {
template <class... Tp>
struct common_type {};

template <class Tp, class Up, class = void>
struct common_type3 {};

template <class Tp, class Up>
using cond_type = decltype( false ? std::declval<Tp>() : std::declval<Up>() );

template <class Tp, class Up>
struct common_type3<Tp, Up, std::void_t<cond_type<const Tp&, const Up&>>> {
    using type = std::remove_cvref_t<cond_type<const Tp&, const Up&>>;
};

template <class Tp, class Up, class = void>
struct common_type2 : common_type3<Tp, Up> {};

template <class Tp, class Up>
struct common_type2<Tp, Up, std::void_t<cond_type<Tp, Up>>> {
    using type = std::decay_t<cond_type<Tp, Up>>;
};

template <class... Tp>
struct common_types {};

template <class... Tp>
struct common_type;

// TODO: why use false?: and true?: both
template <class, class = void>
struct common_type_impl {};

template <class Tp, class Up>
struct common_type_impl<common_types<Tp, Up>, std::void_t<typename common_type<Tp, Up>::type>> {
    using type = typename common_type<Tp, Up>::type;
};

template <class Tp, class Up, class Vp, class... Rest>
struct common_type_impl<common_types<Tp, Up, Vp, Rest...>, std::void_t<typename common_type<Tp, Up>::type>>
    : common_type_impl<common_types<typename common_type<Tp, Up>::type, Vp, Rest...>> {};

// bullet1  sizeof...(Tp) == 0
template <>
struct common_type<> {};

// bullet2 sizeof...(Tp) == 1
template <class Tp>
struct common_type<Tp> : common_type<Tp, Tp> {};

// bullet3 sizeof...(Tp) == 2
template <class Tp, class Up>
struct common_type<Tp, Up> : std::conditional_t<std::is_same_v<Tp, std::decay_t<Tp>> && std::is_same_v<Up, std::decay_t<Up>>, common_type2<Tp, Up>,
                                                common_type<std::decay_t<Tp>, std::decay_t<Up>>> {};

// bullet4 sizeof...(Tp) > 2
template <class Tp, class Up, class Vp, class... Rest>
struct common_type<Tp, Up, Vp, Rest...> : common_type_impl<common_types<Tp, Up, Vp, Rest...>> {};

template <class... Tp>
using common_type_t = typename common_type<Tp...>::type;

}  // namespace wyne

#endif