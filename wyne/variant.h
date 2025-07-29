#ifndef WYNE_VARIANT_H__
#define WYNE_VARIANT_H__

#include "type_traits.h"
#include "type_traits/integer_sequence.h"
#include <exception>

namespace wyne {
// MACRO AUTO
#define AUTO auto
#define AUTO_RETURN( ... )  \
    {                       \
        return __VA_ARGS__; \
    }

#define AUTO_FR auto&&
#define AUTO_FR_RETURN( ... ) \
    {                         \
        return __VA_ARGS__;   \
    }

#define AUTO_DECLTYPE decltype( auto )
#define AUTO_DECLTYPE_RETURN( ... ) \
    {                               \
        return __VA_ARGS__;         \
    }

// bad variant access
class bad_variant_access : public std::exception {
public:
    virtual const char* what() const noexcept override { return "bad variant access"; }

    [[noreturn]] void throw_bad_variant_access() const { throw bad_variant_access(); }
};

// variant_size
template <class T>
struct variant_size;

template <class... Ts>
struct variant;

template <class... Ts>
struct variant_size<variant<Ts...>> : size_constant<sizeof...( Ts )> {};

template <class T>
struct variant_size<const T> : variant_size<T> {};

template <class T>
struct variant_size<volatile T> : variant_size<T> {};

template <class T>
struct variant_size<const volatile T> : variant_size<T> {};

template <class T>
using variant_size_t = variant_size<T>::value;

// variant_alternative <I, Ts...>

template <std::size_t I, class T>
struct variant_alternative;

template <std::size_t I, class... Ts>
struct variant_alternative<I, variant<Ts...>> {
    static_assert( I < sizeof...( Ts ), "index out of bounds in wyne::variant_alternative<>" );
    using type = type_pack_element_t<I, Ts...>;
};

template <std::size_t I, class T>
struct variant_alternative<I, const T> : add_const<variant_alternative<I, T>> {};

template <std::size_t I, class T>
struct variant_alternative<I, volatile T> : add_volatile<variant_alternative<I, T>> {};

template <std::size_t I, class T>
struct variant_alternative<I, const volatile T> : add_cv<variant_alternative<I, T>> {};

template <std::size_t I, class T>
using variant_alternative_t = typename variant_alternative<I, T>::type;

constexpr std::size_t variant_npos = static_cast<std::size_t>( -1 );

// TODO: volatile ???

template <class... Args>
class variant {};
}  // namespace wyne

#endif