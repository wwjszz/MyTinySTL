#ifndef WYNE_OPTIONAL_H__
#define WYNE_OPTIONAL_H__

#include <compare>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "construct.h"
#include "util.h"

// Implementation inspired by folly's Optional
// Does not support constexpr

namespace wyne {
template <class Value>
class optional;

template <class T>
concept is_derived_from_optional = requires( const T& t ) { []<class U>( const optional<U>& ) {}( t ); };

// like std::nullopt_t
struct None {
    enum class secret { token };
    constexpr explicit None( secret ) {}
};
inline constexpr None none{ None::secret::token };

// exception
class optional_empty_exception : public std::runtime_error {
public:
    optional_empty_exception() : std::runtime_error( "empty optional cannot be unwrapped" ) {}
};

template <class Value>
class optional {
public:
    using value_type = Value;

    static_assert( !std::is_reference_v<Value>, "optional may not be used with reference types" );
    static_assert( !std::is_abstract_v<Value>, "optional may not be used with abstract types" );

    // optional(),(const optional&),(optional&&)
    constexpr optional() noexcept : storage() {}

    constexpr optional( const optional& src ) noexcept( std::is_nothrow_copy_constructible_v<Value> ) {
        if ( src.has_value() )
            construct( src.value() );
    }

    constexpr optional( optional&& src ) noexcept( std::is_nothrow_move_constructible_v<Value> ) {
        if ( src.has_value() )
            construct( wyne::move( src.value() ) );
    }

    // optional(const None&), (const Value&), (Value&&)
    constexpr optional( const None& ) noexcept : optional() {}
    constexpr optional( const Value& new_value ) noexcept( std::is_nothrow_copy_constructible_v<Value> ) { construct( new_value ); }
    constexpr optional( Value&& new_value ) noexcept( std::is_nothrow_move_constructible_v<Value> ) { construct( wyne::move( new_value ) ); }

    // optional(in_place, ...)
    // To disambiguate empty optional from default-constructed value
    template <class... Args>
    constexpr optional( std::in_place_t, Args&&... args ) noexcept( std::is_nothrow_constructible_v<Value, Args...> )
        : optional( Constructor{}, wyne::forward<Args>( args )... ) {}

    template <class U, class... Args>
    constexpr optional( std::in_place_t, std::initializer_list<U> il,
                        Args&&... args ) noexcept( std::is_nothrow_constructible_v<Value, std::initializer_list<U>, Args...> )
        : optional( Constructor{}, il, wyne::forward<Args>( args )... ) {}

    // assign None, optional&&, optioanl
    constexpr void assign( const None& ) noexcept { reset(); }

    constexpr void assign( const optional& src ) noexcept( std::is_nothrow_copy_constructible_v<Value> && std::is_nothrow_copy_assignable_v<Value> ) {
        if ( this != &src ) {
            if ( src.has_value() ) {
                assign( src.value() );
            }
            else {
                reset();
            }
        }
    }

    constexpr void assign( optional&& src ) noexcept( std::is_nothrow_move_constructible_v<Value> && std::is_nothrow_move_assignable_v<Value> ) {
        if ( this != &src ) {
            if ( src.has_value() ) {
                assign( wyne::move( src.value() ) );
                src.reset();
            }
            else {
                reset();
            }
        }
    }

    // assign Value&&, const Value&
    constexpr void assign( const Value& new_value ) noexcept( std::is_nothrow_copy_constructible_v<Value>
                                                              && std::is_nothrow_copy_assignable_v<Value> ) {
        if ( has_value() ) {
            storage.value = new_value;
        }
        else {
            construct( new_value );
        }
    }

    constexpr void assign( Value&& new_value ) noexcept( std::is_nothrow_move_constructible_v<Value> && std::is_nothrow_move_assignable_v<Value> ) {
        if ( has_value() ) {
            storage.value = wyne::move( new_value );
        }
        else {
            construct( wyne::move( new_value ) );
        }
    }

    // opreator=
    constexpr optional& operator=( const None& ) noexcept {
        reset();
        return *this;
    }

    constexpr optional& operator=( const optional& other ) noexcept( std::is_nothrow_copy_assignable_v<Value> ) {
        if ( this != &other ) {
            assign( other );
        }
        return *this;
    }

    constexpr optional& operator=( optional&& other ) noexcept( std::is_nothrow_move_assignable_v<Value> ) {
        if ( this != &other ) {
            assign( wyne::move( other ) );
        }
        return *this;
    }

    template <class U>
    constexpr optional& operator=( U&& new_value ) noexcept( std::is_nothrow_assignable_v<Value, U&&> ) {
        assign( wyne::forward<U>( new_value ) );
        return *this;
    }

    template <class... Args>
    Value& emplace( Args&&... args ) noexcept( std::is_nothrow_constructible_v<Value, Args...> ) {
        reset();
        construct( wyne::forward<Args>( args )... );
        return value();
    }

    template <class U, class... Args, class = std::enable_if_t<std::is_constructible_v<Value, std::initializer_list<U>, Args...>>>
    Value& emplace( std::initializer_list<U> il,
                    Args&&... args ) noexcept( std::is_nothrow_constructible_v<Value, std::initializer_list<U>, Args...> ) {
        reset();
        construct( il, wyne::forward<Args>( args )... );
        return value();
    }

    // reset
    constexpr void reset() noexcept { storage.clear(); }

    // clear
    constexpr void clear() noexcept { reset(); }

    // swap
    constexpr void swap( optional& other ) noexcept( std::is_nothrow_swappable_v<Value> ) {
        if ( has_value() && other.has_value() ) {
            using wyne::swap;
            swap( value(), other.value() );
        }
        else if ( has_value() ) {
            other.assign( wyne::move( value() ) );
            reset();
        }
        else {
            assign( wyne::move( other.value() ) );
            other.reset();
        }
    }

    // value
    constexpr Value& value() & {
        required_value();
        return storage.value;
    }

    constexpr const Value& value() const& {
        required_value();
        return storage.value;
    }

    constexpr Value&& value() && {
        required_value();
        return wyne::move( storage.value );
    }

    // get_pointer
    constexpr Value*       get_pointer() & noexcept { return has_value() ? &value() : nullptr; }
    constexpr const Value* get_pointer() const& noexcept { return has_value() ? &value() : nullptr; }
    constexpr Value*       get_pointer() && = delete;

    // has_value
    constexpr bool has_value() const noexcept { return storage.has_value; }

    // bool()
    constexpr explicit operator bool() const noexcept { return has_value(); }

    // operator*
    constexpr Value&       operator*() & noexcept { return value(); }
    constexpr const Value& operator*() const& noexcept { return value(); }
    constexpr Value&&      operator*() && noexcept { return wyne::move( value() ); }

    // operator->
    constexpr Value*       operator->() const noexcept { return &value(); }
    constexpr const Value* operator->() noexcept { return &value(); }

    // value_or
    template <class U>
    constexpr Value value_or( U&& df ) const& noexcept {
        if ( has_value() ) {
            return storage.value;
        }
        return wyne::forward<U>( df );
    }

    template <class U>
    constexpr Value value_or( U&& df ) && noexcept {
        if ( has_value() ) {
            return wyne::move( storage.value );
        }
        return wyne::forward<U>( df );
    }

private:
    template <class U>
    friend constexpr optional<std::decay_t<U>> make_optional( U&& ) noexcept( std::is_nothrow_constructible_v<std::decay_t<U>, U> );
    template <class T, class... Args>
    friend constexpr optional<T> make_optional( Args&&... ) noexcept( std::is_nothrow_constructible_v<T, Args...> );
    template <class T, class U, class... Args>
    friend constexpr optional<T> make_optional( std::initializer_list<U>,
                                                Args&&... ) noexcept( std::is_nothrow_constructible_v<T, std::initializer_list<U>, Args...> );

    // Constructor
    struct Constructor {
        explicit Constructor() = default;
    };

    template <class... Args>
    constexpr optional( Constructor, Args&&... args ) noexcept( std::is_nothrow_constructible_v<Value, Args...> ) {
        construct( wyne::forward<Args>( args )... );
    }

    constexpr void required_value() const {
        if ( !storage.has_value )
            throw optional_empty_exception();
    }

    template <class... Args>
    constexpr void construct( Args&&... args ) noexcept( std::is_nothrow_constructible_v<Value, Args...> ) {
        wyne::construct_at( &storage.value, wyne::forward<Args>( args )... );
        storage.has_value = true;
    }

    struct Storage {
        union {
            char       dummy;
            value_type value;
        };
        bool has_value;

        constexpr Storage() noexcept : dummy(), has_value( false ) {}
        constexpr ~Storage() noexcept { clear(); }

        constexpr void clear() noexcept {
            if ( has_value ) {
                has_value = false;
                value.~Value();
            }
        }
    };
    Storage storage;
};

// swap
template <class Value>
constexpr void swap( optional<Value>& x, optional<Value>& y ) noexcept( noexcept( x.swap( y ) ) ) {
    x.swap( y );
}

// make_optional
template <class U>
constexpr optional<std::decay_t<U>> make_optional( U&& u ) noexcept( std::is_nothrow_constructible_v<std::decay_t<U>, U> ) {
    using Constructor = optional<std::decay_t<U>>::Constructor;
    return { Constructor{}, wyne::forward<U>( u ) };
}

template <class T, class... Args>
constexpr optional<T> make_optional( Args&&... args ) noexcept( std::is_nothrow_constructible_v<T, Args...> ) {
    using Constructor = optional<T>::Constructor;
    return { Constructor{}, wyne::forward<Args>( args )... };
}

template <class T, class U, class... Args>
constexpr optional<T> make_optional( std::initializer_list<U> il,
                                     Args&&... args ) noexcept( std::is_nothrow_constructible_v<T, std::initializer_list<U>, Args...> ) {
    using Constructor = optional<T>::Constructor;
    return { Constructor{}, il, wyne::forward<Args>( args )... };
}

// operator <=>
template <class T, std::three_way_comparable_with<T> U>
constexpr std::compare_three_way_result_t<T, U> operator<=>( const optional<T>& x,
                                                             const optional<U>& y ) noexcept( noexcept( x.value() <=> y.value() ) ) {
    if ( x && y )
        return x.value() <=> y.value();
    return x.has_value() <=> y.has_value();
}

template <class T, class U>
    requires( !is_derived_from_optional<U> ) && std::three_way_comparable_with<T, U>
constexpr std::compare_three_way_result_t<T, U> operator<=>( const optional<T>& x, const U& y ) noexcept( noexcept( x.value() <=> y ) ) {
    return x.has_value() ? x.value() <=> y : std::strong_ordering::less;
}

template <class T>
constexpr std::strong_ordering operator<=>( const optional<T>& x, None ) noexcept {
    return x.has_value() <=> false;
}

// operator ==
template <class T, class U>
constexpr std::enable_if_t<std::is_convertible_v<decltype( std::declval<const T&>() == std::declval<const U&>() ), bool>, bool>
operator==( const optional<T>& x, const optional<U>& y ) noexcept( noexcept( x.value() == y.value() ) ) {
    if ( static_cast<bool>( x ) != static_cast<bool>( y ) )
        return false;
    if ( !x )
        return true;
    return x.value() == y.value();
}

template <class T, class U>
constexpr std::enable_if_t<std::is_convertible_v<decltype( std::declval<const T&>() == std::declval<const U&>() ), bool>, bool>
operator==( const optional<T>& x, const U& y ) noexcept( noexcept( x.value() == y ) ) {
    return x.has_value() ? x.value() == y : false;
}

template <class T>
constexpr bool operator==( const optional<T>& x, None ) noexcept {
    return !x.has_value();
}

}  // namespace wyne

#endif