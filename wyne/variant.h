#ifndef WYNE_VARIANT_H__
#define WYNE_VARIANT_H__

#include <array>
#include <compare>
#include <concepts>
#include <exception>
#include <functional>
#include <initializer_list>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

#include "config.h"
#include "type_traits.h"
#include "type_traits/concepts.h"
#include "type_traits/in_place.h"
#include "type_traits/integer_sequence.h"
#include "type_traits/type_pack_element.h"
#include "util.h"

namespace wyne {

#define WYNE_SWITCH( I, B, CASE, DEFAULT ) \
    switch ( ( I ) ) {                     \
        CASE( B + 0 );                     \
        CASE( B + 1 );                     \
        CASE( B + 2 );                     \
        CASE( B + 3 );                     \
        CASE( B + 4 );                     \
        CASE( B + 5 );                     \
        CASE( B + 6 );                     \
        CASE( B + 7 );                     \
        CASE( B + 8 );                     \
        CASE( B + 9 );                     \
        CASE( B + 10 );                    \
        CASE( B + 11 );                    \
        CASE( B + 12 );                    \
        CASE( B + 13 );                    \
        CASE( B + 14 );                    \
        CASE( B + 15 );                    \
        CASE( B + 16 );                    \
        CASE( B + 17 );                    \
        CASE( B + 18 );                    \
        CASE( B + 19 );                    \
        CASE( B + 20 );                    \
        CASE( B + 21 );                    \
        CASE( B + 22 );                    \
        CASE( B + 23 );                    \
        CASE( B + 24 );                    \
        CASE( B + 25 );                    \
        CASE( B + 26 );                    \
        CASE( B + 27 );                    \
        CASE( B + 28 );                    \
        CASE( B + 29 );                    \
        CASE( B + 30 );                    \
        CASE( B + 31 );                    \
        DEFAULT( B + 32 );                 \
    }

// TODO: cpp standard > 11
#define WYNE_INHERITING_CTOR( type, base ) using base::base;

// bad variant access
class bad_variant_access : public std::exception {
public:
    virtual const char* what() const noexcept override { return "bad variant access"; }
};

[[noreturn]] inline void throw_bad_variant_access() { throw bad_variant_access(); }

// variant_size
template <class T>
struct variant_size;

template <class... Ts>
class variant;

template <class T>
concept is_variant = requires( T t ) {
    {
        []<class... Ts>( variant<Ts...> ) {}( t )
    };
};

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

// T(From<Ts...>) -> To<Ts...>
template <class T, template <class...> class To, class... Res>
struct unpack_impl {};

template <template <class...> class From, template <class...> class To, class... Ts, class... Res>
struct unpack_impl<From<Ts...>, To, Res...> {
    using type = To<Res..., Ts...>;
};

template <class T, template <class...> class To, class... Res>
using unpack_to = unpack_impl<T, To, Res...>::type;

constexpr std::size_t variant_npos = static_cast<std::size_t>( -1 );

// TODO: volatile ???

namespace core {

    constexpr std::size_t not_found = static_cast<std::size_t>( -1 );
    constexpr std::size_t ambiguous = static_cast<std::size_t>( -2 );

    template <class T, class... Ts>
    inline constexpr std::size_t find_index() {
        constexpr std::array<bool, sizeof...( Ts )> same   = { std::is_same_v<T, Ts>... };
        std::size_t                                 result = not_found;
        for ( std::size_t i = 0; i < sizeof...( Ts ); ++i ) {
            if ( same[ i ] ) {
                if ( result != not_found ) {
                    return ambiguous;  // found more than one
                }
                result = i;
            }
        }
        return result;
    }

    template <std::size_t I>
    using find_index_sfinae_impl = std::enable_if_t<I != not_found && I != ambiguous, size_constant<I>>;

    template <class T, class... Ts>
    using find_index_sfinae = find_index_sfinae_impl<find_index<T, Ts...>()>;

    template <class T, class... Ts>
    inline constexpr std::size_t find_index_sfinae_t = find_index_sfinae<T, Ts...>::value;

    template <std::size_t I>
    struct find_index_checked_impl : size_constant<I> {
        static_assert( I != not_found, "the specified type is not found." );
        static_assert( I != ambiguous, "the specified type is ambiguous." );
    };

    template <class T, class... Ts>
    using find_index_checked = find_index_checked_impl<find_index<T, Ts...>()>;

    template <class T, class... Ts>
    inline constexpr std::size_t find_index_checked_t = find_index_checked<T, Ts...>::value;

    struct valueless_t {};

    // Sorted by priority
    enum class Trait : unsigned char { TrivallyAvailable = 0, Available = 1, Unavailable = 2 };

    template <class T, template <class> class IsTrivallyAvailable, template <class> class IsAvailable>
    inline constexpr Trait trait() noexcept {
        return IsTrivallyAvailable<T>::value ? Trait::TrivallyAvailable : IsAvailable<T>::value ? Trait::Available : Trait::Unavailable;
    }

    template <class... Traits>
        requires wyne::all<std::is_same_v<Traits, Trait>...>
    inline constexpr Trait common_traits( Traits... traits ) noexcept {
        Trait                                  result  = Trait::TrivallyAvailable;
        std::array<Trait, sizeof...( Traits )> _traits = { traits... };
        for ( std::size_t i = 0; i < sizeof...( Traits ); ++i ) {
            Trait t = _traits[ i ];
            if ( static_cast<int>( t ) > static_cast<int>( result ) ) {
                result = t;
            }
        }
        return result;
    }

    template <class... Ts>
    struct traits;

    template <class T>
    inline constexpr bool is_traits_v = false;

    template <class... Ts>
    inline constexpr bool is_traits_v<traits<Ts...>> = true;

    template <class T>
    concept is_traits = is_traits_v<std::remove_cvref_t<T>>;

    template <class... Ts>
    struct traits {
        static constexpr Trait copy_constructible_trait =
            common_traits( trait<Ts, std::is_trivially_copy_constructible, std::is_copy_constructible>()... );

        static constexpr Trait move_constructible_trait =
            common_traits( trait<Ts, std::is_trivially_move_constructible, std::is_move_constructible>()... );

        static constexpr Trait copy_assignable_trait = common_traits( trait<Ts, std::is_trivially_copy_assignable, std::is_copy_assignable>()... );

        static constexpr Trait move_assignable_trait = common_traits( trait<Ts, std::is_trivially_move_assignable, std::is_move_assignable>()... );

        static constexpr Trait destructible_trait = common_traits( trait<Ts, std::is_trivially_destructible, std::is_destructible>()... );
    };

    namespace access {

        struct recursive_union {
            template <class R>
            static constexpr WYNE_AUTO_FR get_alt( R&& ru, in_place_index_t<0> ) noexcept WYNE_AUTO_FR_RETURN( wyne::forward<R>( ru ).head_ );

            template <class R, std::size_t I>
            static constexpr WYNE_AUTO_FR get_alt( R&& ru, in_place_index_t<I> ) noexcept
                WYNE_AUTO_FR_RETURN( get_alt( wyne::forward<R>( ru ).tail_, in_place_index_t<I - 1>{} ) );
        };

        struct base {
            template <std::size_t I, class Impl>
            static constexpr WYNE_AUTO_FR get_alt( Impl&& impl ) noexcept
                WYNE_AUTO_FR_RETURN( recursive_union::get_alt( data( wyne::forward<Impl>( impl ) ), in_place_index_t<I>{} ) );
        };

        struct variant {
            template <std::size_t I, class V>
            static constexpr WYNE_AUTO_FR get_alt( V&& v ) noexcept WYNE_AUTO_FR_RETURN( base::get_alt<I>( wyne::forward<V>( v ).impl_ ) );
        };

    }  // namespace access

    namespace visitation {
        //  HACK: ITs contains reference?
        template <class Ret, class F, class... ITs>
        concept dispatch_return_check = wyne::return_as<Ret, F, decltype( access::base::get_alt<ITs::value>( std::declval<ITs::type>() ) )...>;

        struct base {
            template <class Visitor, class... Vs>
            using dispatch_result_t = std::invoke_result_t<Visitor, decltype( access::base::get_alt<0>( std::declval<Vs>() ) )...>;

            template <class Ret, class F, class... ITs>
            struct return_check_helper {
                static_assert( dispatch_return_check<Ret, F, ITs...>, "`visit` requires the visitor to have a single return type" );
                template <class... Alts>
                static constexpr Ret invoke( F&& f, Alts&&... alts )
                    WYNE_NOEXCEPT_RETURN( std::invoke( wyne::forward<F>( f ), wyne::forward<Alts>( alts )... ) );
            };

            // ITs: indexed_type<0, Bt>, indexed_type<1, Bt>, ..., indexed_type<N, Bt>
            // where Bt is the base type of the variant.
            template <bool B, class Ret, class... ITs>
            struct dispatcher;

            template <class Ret, class... ITs>
            struct dispatcher<false, Ret, ITs...> {};

            template <class Ret, class... ITs>
            struct dispatcher<true, Ret, ITs...> {
                template <class F>
                static constexpr Ret dispatch( F&& f, typename ITs::type&&... visited_bs )
                    WYNE_NOEXCEPT_RETURN( return_check_helper<Ret, F, ITs...>::invoke(
                        wyne::forward<F>( f ), access::base::get_alt<ITs::value>( wyne::forward<typename ITs::type>( visited_bs ) )... ) );

                template <std::size_t B, class F, class Bt, class... Bs>
                static constexpr Ret dispatch( F&& f, typename ITs::type&&... visited_bs, Bt&& b, Bs&&... bs ) {
                    // TODO: V::size() -> std::decay_t<V>::size() ?
#define WYNE_DISPATCH( I )                                                                                                         \
    dispatcher<( I < Bt::size() ), Ret, ITs..., indexed_type<I, Bt>>::template dispatch<0>(                                        \
        wyne::forward<F>( f ), wyne::forward<typename ITs::type>( visited_bs )..., wyne::forward<Bt>( b ), wyne::forward<Bt>( b ), \
        wyne::forward<Bs>( bs )... )

#define WYNE_DEFAULT( I )                                                                                                                         \
    dispatcher<( I < Bt::size() ), Ret, ITs...>::template dispatch<I>( wyne::forward<F>( f ), wyne::forward<typename ITs::type>( visited_bs )..., \
                                                                       wyne::forward<Bt>( b ), wyne::forward<Bt>( b ), wyne::forward<Bs>( bs )... )

#define WYNE_CASE( I ) \
    case I:            \
        return WYNE_DISPATCH( I )
#define WYNE_DEFAULT_CASE( I ) \
    default:                   \
        return WYNE_DEFAULT( I )

                    WYNE_SWITCH( b.index(), B, WYNE_CASE, WYNE_DEFAULT_CASE );

#undef WYNE_DEFAULT_CASE
#undef WYNE_CASE
#undef WYNE_DEFAULT
#undef WYNE_DISPATCH
                }

                template <std::size_t I, class F, class... Bs>
                static constexpr Ret dispatch_case( F&& f, Bs&&... bs )
                    WYNE_NOEXCEPT_RETURN( return_check_helper<Ret, F, ITs...>::invoke( wyne::forward<F>( f ),
                                                                                       access::base::get_alt<I>( wyne::forward<Bs>( bs ) )... ) );

                template <std::size_t B, class F, class Bt, class... Bs>
                static constexpr Ret dispatch_at( std::size_t index, F&& f, Bt&& b, Bs&&... bs ) {
                    static_assert( all<( Bt::size() == Bs::size() )...>, "all of the variants must be the same size." );

#define WYNE_DISPATCH_AT( I )                                                                                              \
    dispatcher<( I < Bt::size() ), Ret, ITs...>::template dispatch_case<I>( wyne::forward<F>( f ), wyne::forward<Bt>( b ), \
                                                                            wyne::forward<Bs>( bs )... )

#define WYNE_DEFAULT_AT( I )                                                                                                    \
    dispatcher<( I < Bt::size() ), Ret, ITs...>::template dispatch_at<I>( index, wyne::forward<F>( f ), wyne::forward<Bt>( b ), \
                                                                          wyne::forward<Bs>( bs )... )

#define WYNE_CASE( I ) \
    case I:            \
        return WYNE_DISPATCH_AT( I )

#define WYNE_DEFAULT_CASE( I ) \
    default:                   \
        WYNE_DEFAULT_AT( I )

                    WYNE_SWITCH( index, B, WYNE_CASE, WYNE_DEFAULT_CASE );

#undef WYNE_DEFAULT_CASE
#undef WYNE_CASE
#undef WYNE_DEFAULT_AT
#undef WYNE_DISPATCH_AT
                }
            };
        };

        struct alt {
            // TODO: Why use decltype(auto)
            // Is: Impl
            template <class Visitor, class... Is>
            static constexpr WYNE_DECLTYPE_AUTO visit_alt( Visitor&& visitor, Is&&... is )
                // TODO: as_base
                WYNE_NOEXCEPT_RETURN( base::dispatcher<true, base::dispatch_result_t<Visitor, Is...>>::template dispatch<0>(
                    wyne::forward<Visitor>( visitor ), wyne::forward<Is>( is )... ) );

            template <class Visitor, class... Is>
            static constexpr WYNE_DECLTYPE_AUTO visit_alt_at( std::size_t index, Visitor&& visitor, Is&&... is )
                WYNE_NOEXCEPT_RETURN( base::dispatcher<true, base::dispatch_result_t<Visitor, Is...>>::template dispatch_at<0>(
                    index, wyne::forward<Visitor>( visitor ), wyne::forward<Is>( is )... ) );
        };

        // variant
        struct variant {
            // check
            template <class Visitor, class... Args>
            struct visit_exhaustiveness_check {
                static_assert( std::invocable<Visitor, Args...>, "`visit` requires the visitor to be exhaustive." );
                static constexpr WYNE_DECLTYPE_AUTO invoke( Visitor&& visitor, Args&&... args )
                    WYNE_NOEXCEPT_RETURN( std::invoke( wyne::forward<Visitor>( visitor ), wyne::forward<Args>( args )... ) );
            };

            // value_visitor
            template <class Visitor>
            struct value_visitor {
                Visitor&& visitor_;

                template <class... Alts>
                constexpr WYNE_DECLTYPE_AUTO operator()( Alts&&... alts ) const
                    WYNE_NOEXCEPT_RETURN( visit_exhaustiveness_check<Visitor, decltype( wyne::forward<Alts>( alts ).value )...>::invoke(
                        wyne::forward<Visitor>( visitor_ ), wyne::forward<Alts>( alts ).value... ) );
            };

            // make_value_visitor
            template <class Visitor>
            static constexpr WYNE_DECLTYPE_AUTO make_value_visitor( Visitor&& visitor )
                WYNE_NOEXCEPT_RETURN( value_visitor<Visitor>{ wyne::forward<Visitor>( visitor ) } );

            template <class Visitor, class... Vs>
            static constexpr WYNE_DECLTYPE_AUTO visit_alt( Visitor&& visitor, Vs&&... vs )
                WYNE_NOEXCEPT_RETURN( alt::visit_alt( wyne::forward<Visitor>( visitor ), wyne::forward<Vs>( vs ).impl_... ) );

            template <class Visitor, class... Vs>
            static constexpr WYNE_DECLTYPE_AUTO visit_alt_at( std::size_t index, Visitor&& visitor, Vs&&... vs )
                WYNE_NOEXCEPT_RETURN( alt::visit_alt_at( index, wyne::forward<Visitor>( visitor ), wyne::forward<Vs>( vs ).impl_... ) );

            // visit_value visit_value_at
            template <class Visitor, class... Vs>
            static constexpr WYNE_DECLTYPE_AUTO visit_value( Visitor&& visitor, Vs&&... vs )
                WYNE_NOEXCEPT_RETURN( visit_alt( make_value_visitor( wyne::forward<Visitor>( visitor ) ), wyne::forward<Vs>( vs )... ) );

            template <class Visitor, class... Vs>
            static constexpr WYNE_DECLTYPE_AUTO visit_value_at( std::size_t index, Visitor&& visitor, Vs&&... vs )
                WYNE_NOEXCEPT_RETURN( visit_alt_at( index, make_value_visitor( wyne::forward<Visitor>( visitor ) ), wyne::forward<Vs>( vs )... ) );
        };

    }  // namespace visitation

    // alt
    template <std::size_t I, class T>
    struct alt;

    template <class T>
    inline constexpr bool is_alt_v = false;

    template <std::size_t I, class T>
    inline constexpr bool is_alt_v<alt<I, T>> = true;

    template <class T>
    concept is_alt = is_alt_v<std::remove_cvref_t<T>>;

    template <std::size_t I, class T>
    struct alt {
        using value_type = T;

        template <class... Args>
        // std::is_nothrow_constructible_v<T, Args...> 与 Args&&...
        constexpr alt( in_place_t, Args&&... args ) noexcept( std::is_nothrow_constructible_v<T, Args...> )
            : value( wyne::forward<Args>( args )... ) {}

        T value;
    };

    // recursive_union
    template <Trait DestructibleTrait, std::size_t Index, class... Ts>
    union recursive_union;

    template <Trait DestructibleTrait, std::size_t Index>
    union recursive_union<DestructibleTrait, Index> {
        // TODO: UNREACHABLE
    };

#define WYNE_RECURSIVE_UNION( destructible_trait, destructor )                                                                                   \
    template <std::size_t Index, class T, class... Ts>                                                                                           \
    union recursive_union<destructible_trait, Index, T, Ts...> {                                                                                 \
        constexpr recursive_union( valueless_t ) noexcept : dummy_{} {}                                                                          \
        template <class... Args>                                                                                                                 \
        constexpr recursive_union( in_place_index_t<0>, Args&&... args ) : head_( in_place_t{}, wyne::forward<Args>( args )... ) {}              \
        template <std::size_t I, class... Args>                                                                                                  \
        constexpr recursive_union( in_place_index_t<I>, Args&&... args ) : tail_( in_place_index_t<I - 1>{}, wyne::forward<Args>( args )... ) {} \
                                                                                                                                                 \
        recursive_union( const recursive_union& ) = default;                                                                                     \
        recursive_union( recursive_union&& )      = default;                                                                                     \
                                                                                                                                                 \
        destructor;                                                                                                                              \
                                                                                                                                                 \
        recursive_union& operator=( const recursive_union& ) = default;                                                                          \
        recursive_union& operator=( recursive_union&& )      = default;                                                                          \
                                                                                                                                                 \
    private:                                                                                                                                     \
        char                                                  dummy_;                                                                            \
        alt<Index, T>                                         head_;                                                                             \
        recursive_union<destructible_trait, Index + 1, Ts...> tail_;                                                                             \
                                                                                                                                                 \
        friend access::recursive_union;                                                                                                          \
    };

    WYNE_RECURSIVE_UNION( Trait::TrivallyAvailable, ~recursive_union() = default );
    WYNE_RECURSIVE_UNION( Trait::Available, ~recursive_union(){} );
    WYNE_RECURSIVE_UNION( Trait::Unavailable, ~recursive_union() = delete; );

#undef WYNE_RECURSIVE_UNION

    template <Trait DestructibleTrait, class... Ts>
    class base;

    template <class T>
    struct is_base_template : wyne::false_type {};

    template <Trait DestructibleTrait, class... Ts>
    struct is_base_template<base<DestructibleTrait, Ts...>> : wyne::true_type {};

    template <class T>
    concept is_base = is_base_template<std::remove_cvref_t<T>>::value;

    template <class T>
    concept convertible_to_base =
        requires( T&& t ) { []<Trait DestructibleTrait, class... Ts>( base<DestructibleTrait, Ts...>&& ) {}( wyne::forward<T>( t ) ); };

    // index_t
    template <class... Ts>
    using index_t =
        std::conditional_t<sizeof...( Ts ) < std::numeric_limits<unsigned char>::max(), unsigned char,
                           std::conditional_t<sizeof...( Ts ) < std::numeric_limits<unsigned short>::max(), unsigned short, unsigned int>>;

    template <Trait DestructibleTrait, class... Ts>
    class base {
    public:
        constexpr base( valueless_t tag ) noexcept : data_( tag ), index_( static_cast<index_t<Ts...>>( -1 ) ) {}

        template <std::size_t I, class... Args>
        constexpr base( in_place_index_t<I>, Args&&... args ) : data_( in_place_index_t<I>{}, wyne::forward<Args>( args )... ), index_( I ) {}

        constexpr bool valueless_by_exception() const noexcept { return index_ == static_cast<std::size_t>( -1 ); }

        constexpr std::size_t index() const noexcept { return valueless_by_exception() ? variant_npos : index_; }

    protected:
        using data_t = recursive_union<DestructibleTrait, 0, Ts...>;

        friend constexpr base&        as_base( base& impl ) noexcept { return impl; }
        friend constexpr const base&  as_base( const base& impl ) noexcept { return impl; }
        friend constexpr base&&       as_base( base&& impl ) noexcept { return wyne::move( impl ); }
        friend constexpr const base&& as_base( const base&& impl ) noexcept { return wyne::move( impl ); }

        friend constexpr data_t&        data( base& impl ) noexcept { return impl.data_; }
        friend constexpr const data_t&  data( const base& impl ) noexcept { return impl.data_; }
        friend constexpr data_t&&       data( base&& impl ) noexcept { return wyne::move( impl.data_ ); }
        friend constexpr const data_t&& data( const base&& impl ) noexcept { return wyne::move( impl.data_ ); }

        static constexpr std::size_t size() noexcept { return sizeof...( Ts ); }

        data_t         data_;
        index_t<Ts...> index_;
    };

    struct destroyer {
        template <class Alt>
        constexpr void operator()( Alt&& alt ) const noexcept {
            alt.~Alt();
        }
    };

    template <is_traits Traits, Trait = Traits::destructible_trait>
    class destructor;

#define WYNE_DESTRUCTOR( destructible_trait, definition, destroy )                                 \
    template <class... Ts>                                                                         \
    class destructor<traits<Ts...>, destructible_trait> : public base<destructible_trait, Ts...> { \
        using super = base<destructible_trait, Ts...>;                                             \
                                                                                                   \
    public:                                                                                        \
        WYNE_INHERITING_CTOR( destructor, super );                                                 \
        using super::operator=;                                                                    \
                                                                                                   \
        definition;                                                                                \
        destructor( const destructor& ) = default;                                                 \
        destructor( destructor&& )      = default;                                                 \
                                                                                                   \
        destructor& operator=( const destructor& ) = default;                                      \
        destructor& operator=( destructor&& )      = default;                                      \
                                                                                                   \
    protected:                                                                                     \
        destroy;                                                                                   \
    };

    WYNE_DESTRUCTOR(
        Trait::TrivallyAvailable, ~destructor() = default, constexpr void destroy() noexcept { this->index_ = static_cast<index_t<Ts...>>( -1 ); } );

    WYNE_DESTRUCTOR(
        Trait::Available, ~destructor() { destroy(); },
        constexpr void destroy() noexcept {
            if ( !this->valueless_by_exception() ) {
                visitation::alt::visit_alt( destroyer{}, *this );
            }
            this->index_ = static_cast<index_t<Ts...>>( -1 );
        } );

    WYNE_DESTRUCTOR( Trait::Unavailable, ~destructor() = delete, constexpr void destroy() noexcept = delete );

#undef WYNE_DESTRUCTOR

    template <is_traits Traits>
    class constructor : public destructor<Traits> {
        using super = destructor<Traits>;

    public:
        WYNE_INHERITING_CTOR( constructor, super );
        using super::operator=;

    protected:
        template <std::size_t I, class T, class... Args>
        static T& construct_alt( alt<I, T>& a, Args&&... args ) {
            auto* result = ::new ( static_cast<void*>( std::addressof( a ) ) ) alt<I, T>( in_place_t{}, wyne::forward<Args>( args )... );
            return result->value;
        }

        template <convertible_to_base T>
        static void generic_construct( constructor& lhs, T&& rhs ) {
            lhs.destroy();
            if ( !rhs.valueless_by_exception() ) {
                visitation::alt::visit_alt_at(
                    rhs.index(),
                    []( is_alt auto&& lhs_alt, is_alt auto&& rhs_alt ) {
                        constructor::construct_alt( lhs_alt, wyne::forward<decltype( rhs_alt )>( rhs_alt ).value );
                    },
                    lhs, wyne::forward<T>( rhs ) );
                lhs.index_ = rhs.index_;
            }
        }
    };

    template <is_traits Traits, Trait = Traits::move_constructible_trait>
    class move_constructor;

#define WYNE_MOVE_CONSTRUCTOR( move_constructible_trait, definition )                                     \
    template <class... Ts>                                                                                \
    class move_constructor<traits<Ts...>, move_constructible_trait> : public constructor<traits<Ts...>> { \
        using super = constructor<traits<Ts...>>;                                                         \
                                                                                                          \
    public:                                                                                               \
        WYNE_INHERITING_CTOR( move_constructor, super );                                                  \
        using super::operator=;                                                                           \
                                                                                                          \
        move_constructor( const move_constructor& ) = default;                                            \
        definition;                                                                                       \
                                                                                                          \
        move_constructor& operator=( const move_constructor& ) = default;                                 \
        move_constructor& operator=( move_constructor&& )      = default;                                 \
        ~move_constructor()                                    = default;                                 \
    };

    WYNE_MOVE_CONSTRUCTOR( Trait::TrivallyAvailable, move_constructor( move_constructor&& ) = default; );

    // TODO: figure out: template <bool... Bs> using all = std::is_same<integer_sequence<bool, true, Bs...>, integer_sequence<bool, Bs..., true>>;
    WYNE_MOVE_CONSTRUCTOR(
        Trait::Available,
        move_constructor( move_constructor&& that ) noexcept( wyne::all<std::is_nothrow_move_constructible_v<Ts>...> ) : move_constructor(
            valueless_t{} ) { this->generic_construct( *this, wyne::move( that ) ); } );

    WYNE_MOVE_CONSTRUCTOR( Trait::Unavailable, move_constructor( move_constructor&& ) = delete; );

#undef WYNE_MOVE_CONSTRUCTOR

    template <is_traits Traits, Trait = Traits::copy_constructible_trait>
    class copy_constructor;

#define WYNE_COPY_CONSTRUCTOR( copy_constructible_trait, definition )                                          \
    template <class... Ts>                                                                                     \
    class copy_constructor<traits<Ts...>, copy_constructible_trait> : public move_constructor<traits<Ts...>> { \
        using super = move_constructor<traits<Ts...>>;                                                         \
                                                                                                               \
    public:                                                                                                    \
        WYNE_INHERITING_CTOR( copy_constructor, super );                                                       \
        using super::operator=;                                                                                \
                                                                                                               \
        definition;                                                                                            \
        copy_constructor( copy_constructor&& ) = default;                                                      \
                                                                                                               \
        copy_constructor& operator=( const copy_constructor& ) = default;                                      \
        copy_constructor& operator=( copy_constructor&& )      = default;                                      \
        ~copy_constructor()                                    = default;                                      \
    };

    WYNE_COPY_CONSTRUCTOR( Trait::TrivallyAvailable, copy_constructor( const copy_constructor& ) = default );

    WYNE_COPY_CONSTRUCTOR(
        Trait::Available,
        copy_constructor( const copy_constructor& that ) noexcept( wyne::all<std::is_nothrow_copy_constructible_v<Ts>...> ) : copy_constructor(
            valueless_t{} ) { this->generic_construct( *this, that ); } );

    WYNE_COPY_CONSTRUCTOR( Trait::Unavailable, copy_constructor( const copy_constructor& ) = delete );

#undef WYNE_COPY_CONSTRUCTOR

    // assignment emplace assign_alt generic_assign
    template <is_traits Traits>
    class assignment : public copy_constructor<Traits> {
        using super = copy_constructor<Traits>;

    public:
        WYNE_INHERITING_CTOR( assignment, super );
        using super::operator=;

        template <std::size_t I, class... Args>
        constexpr decltype( auto ) emplace( Args&&... args ) noexcept {
            this->destroy();
            // TODO: handle exception
            auto& result = this->construct_alt( access::base::get_alt<I>( *this ), wyne::forward<Args>( args )... );

            this->index_ = I;
            return result;
        }

    protected:
        template <std::size_t I, class T, class Args>
        void assign_alt( alt<I, T>& a, Args&& args ) noexcept( std::is_nothrow_assignable_v<T, Args> ) {
            if ( this->index() == I ) {
                a.value = T( wyne::forward<Args>( args ) );
            }
            else {
                this->emplace<I>( wyne::forward<Args>( args ) );
            }
        }

        // TODO: concept
        template <std::convertible_to<assignment> T>
        void generic_assign( T&& rhs ) {
            if ( this->valueless_by_exception() && rhs.valueless_by_exception() ) {
                // do nothing
            }
            else if ( rhs.valueless_by_exception() ) {
                this->destroy();
            }
            else {
                visitation::alt::visit_alt_at(
                    rhs.index(),
                    [ this ]( is_alt auto&& lhs_alt, is_alt auto&& rhs_alt ) {
                        this->assign_alt( lhs_alt, wyne::forward<decltype( rhs_alt )>( rhs_alt ).value );
                    },
                    *this, wyne::forward<T>( rhs ) );
            }
        }
    };

    template <class Traits, Trait = Traits::move_assignable_trait>
    class move_assignment;

#define WYNE_MOVE_ASSIGNMENT( move_assignment_trait, definition )                                    \
    template <class... Ts>                                                                           \
    class move_assignment<traits<Ts...>, move_assignment_trait> : public assignment<traits<Ts...>> { \
        using super = assignment<traits<Ts...>>;                                                     \
                                                                                                     \
    public:                                                                                          \
        WYNE_INHERITING_CTOR( move_assignment, super );                                              \
        using super::operator=;                                                                      \
                                                                                                     \
        move_assignment( const move_assignment& ) = default;                                         \
        move_assignment( move_assignment&& )      = default;                                         \
                                                                                                     \
        move_assignment& operator=( const move_assignment& ) = default;                              \
        definition;                                                                                  \
        ~move_assignment() = default;                                                                \
    };

    WYNE_MOVE_ASSIGNMENT( Trait::TrivallyAvailable, move_assignment& operator=( move_assignment&& ) = default );

    WYNE_MOVE_ASSIGNMENT(
        Trait::Available, move_assignment& operator=( move_assignment&& that ) noexcept(
                              wyne::all<( std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_move_assignable_v<Ts> )...> ) {
            this->generic_assign( wyne::move( that ) );
        } );

    WYNE_MOVE_ASSIGNMENT( Trait::Unavailable, move_assignment& operator=( move_assignment&& ) = delete );

#undef WYNE_MOVE_ASSIGNMENT

    template <class... Ts>
    class impl : public move_assignment<traits<Ts...>> {
        using super = move_assignment<traits<Ts...>>;

    public:
        WYNE_INHERITING_CTOR( impl, super );
        using super::operator=;

        impl( const impl& )            = default;
        impl( impl&& )                 = default;
        impl& operator=( const impl& ) = default;
        impl& operator=( impl&& )      = default;

        template <std::size_t I, class T>
        constexpr void assign( T&& t ) noexcept( noexcept( this->assign_alt( access::base::get_alt<I>( *this ), wyne::forward<T>( t ) ) ) ) {
            this->assign_alt( access::base::get_alt<I>( *this ), wyne::forward<T>( t ) );
        }

        constexpr void swap( impl& that ) noexcept {
            if ( this->valueless_by_exception() && that.valueless_by_exception() ) {
                // do nothing
            }
            else if ( this->index_ == that.index_ ) {
                visitation::alt::visit_alt_at(
                    this->index_,
                    []( auto& lhs_alt, auto& rhs_alt ) {
                        using std::swap;
                        swap( lhs_alt, rhs_alt );
                    },
                    *this, that );
            }
            else {
                // TODO: Exception
                impl temp( wyne::move( that ) );
                this->generic_construct( that, wyne::move( *this ) );
                this->generic_construct( *this, wyne::move( temp ) );
            }
        }

    private:
        constexpr bool move_nothrow() noexcept {
            return this->valueless_by_exception() || std::array<bool, sizeof...( Ts )>{ std::is_nothrow_move_constructible_v<Ts>... }[ this->index_ ];
        }
    };

#undef WYNE_INHERITING_CTOR

    template <class From, class To>
    concept convert_without_narrowing = requires( From&& from ) {
        { std::type_identity_t<To[]>{ from } };
    };

    template <class T>
    concept is_arithemetic = std::is_arithmetic_v<T>;

    template <std::size_t I, class T>
    struct overload_leaf_helper {
        using impl = size_constant<I> ( * )( T );
        operator impl() const { return nullptr; }
    };

    template <class Arg, std::size_t I, class T>
    struct overload_leaf {};

    template <class Arg, std::size_t I, class T>
        requires( !is_arithemetic<T> )
    struct overload_leaf<Arg, I, T> : overload_leaf_helper<I, T> {};

    template <class Arg, std::size_t I, is_arithemetic T>
        requires( std::is_same_v<std::remove_cvref_t<T>, bool> ? std::is_same_v<std::remove_cvref_t<Arg>, bool> : convert_without_narrowing<Arg, T> )
    struct overload_leaf<Arg, I, T> : overload_leaf_helper<I, T> {};

    template <class Arg, class... Ts>
    struct overload_impl {
    private:
        template <class T>
        struct impl;

        template <std::size_t... Is>
        struct impl<index_sequence<Is...>> : overload_leaf<Arg, Is, Ts>... {};

    public:
        using type = impl<index_sequence_for<Ts...>>;
    };

    template <class Arg, class... Ts>
    using overload = overload_impl<Arg, Ts...>::type;

    template <class Arg, class... Ts>
        requires requires( Arg arg ) {
            { overload<Arg, Ts...>{}( arg ) };
        }
    using best_match = std::invoke_result_t<overload<Arg, Ts...>, Arg>;

    template <class Arg, class... Ts>
    inline constexpr std::size_t best_match_v = best_match<Arg, Ts...>::value;

    template <class T>
    struct is_in_place_index_impl : false_type {};

    template <std::size_t I>
    struct is_in_place_index_impl<in_place_index_t<I>> : true_type {};

    template <class T>
    concept is_in_place_index = is_in_place_index_impl<T>::value;

    template <class T>
    struct is_in_place_type_impl : false_type {};

    template <class T>
    struct is_in_place_type_impl<in_place_type_t<T>> : true_type {};

    template <class T>
    concept is_in_place_type = is_in_place_type_impl<T>::value;

    // static_assert( std::same_as<size_constant<0>, best_match<int, long long>> );

    // static_assert( convert_without_narrowing<bool, int> );

    // static_assert( convertible_to_base<assignment<traits<int>>>, "" );

}  // namespace core

template <class... Ts>
class variant {
    static_assert( 0 < sizeof...( Ts ), "variant must consist of at least one alternative." );

    static_assert( wyne::all<!std::is_array_v<Ts>...>, "variant can not have a array type as an alternative." );

    static_assert( wyne::all<!std::is_reference_v<Ts>...>, "variant can not have a reference type as an alternative." );

    static_assert( wyne::all<!std::is_void_v<Ts>...>, "variant can not have a void type as an alternative." );

    // ~variant opreator=

    // variant(in_place_index_t<I>,std::initializer_list<Up> il,Args &&...args)
    // variant(in_place_type_t<T>,Args &&...args) variant(in_place_type_t<T>,std::initializer_list<Up> il,Args &&...args)
public:
    template <default_constructible First = type_pack_element_t<0, Ts...>>
    constexpr variant() noexcept( std::is_nothrow_default_constructible_v<First> ) : impl_( in_place_index_t<0>{} ) {}

    variant( const variant& ) = default;
    variant( variant&& )      = default;

    template <class Arg,
              // class Decay = std::decay<Arg>  ,
              std::enable_if_t<!core::is_in_place_index<Arg>, int> = 0, std::enable_if_t<!core::is_in_place_type<Arg>, int> = 0,
              std::size_t I = core::best_match_v<Arg, Ts...>, class T = type_pack_element_t<I, Ts...>>
    constexpr variant( Arg&& arg ) noexcept( std::is_nothrow_constructible_v<T, Arg> ) : impl_( in_place_index_t<I>{}, wyne::forward<Arg>( arg ) ) {}

    template <std::size_t I, class... Args, constructible<Args...> T = type_pack_element_t<I, Ts...>>
    constexpr variant( in_place_index_t<I>, Args&&... args ) noexcept( std::is_nothrow_constructible_v<T, Args...> )
        : impl_( in_place_index_t<I>{}, wyne::forward<Args>( args )... ) {}

    template <std::size_t I, class U, class... Args, constructible<std::initializer_list<U>, Args...> T = type_pack_element_t<I, Ts...>>
    constexpr variant( in_place_index_t<I>, std::initializer_list<U> il,
                       // TODO: std::initializer_list<U> -> std::initializer_list<U>& ?
                       Args&&... args ) noexcept( std::is_nothrow_constructible_v<T, std::initializer_list<U>, Args...> )
        : impl_( in_place_index_t<I>{}, il, wyne::forward<Args>( args )... ) {}

    template <class T, class... Args, std::size_t I = core::find_index_sfinae_t<T, Ts...>>
        requires constructible<T, Args...>
    constexpr variant( in_place_type_t<T>, Args&&... args ) noexcept( std::is_nothrow_constructible_v<T, Args...> )
        : impl_( in_place_index_t<I>{}, wyne::forward<Args>( args )... ) {}

    template <class T, class U, class... Args, std::size_t I = core::find_index_sfinae_t<T, Ts...>>
        requires constructible<T, Args...>
    constexpr variant( in_place_type_t<T>, std::initializer_list<U> il,
                       Args&&... args ) noexcept( std::is_nothrow_constructible_v<T, std::initializer_list<U>, Args...> )
        : impl_( in_place_index_t<I>{}, il, wyne::forward<Args>( args )... ) {}

    ~variant() = default;

    variant& operator=( const variant& ) = default;
    variant& operator=( variant&& )      = default;

    template <class Arg, std::size_t I = core::best_match_v<Arg, Ts...>, class T = type_pack_element_t<I, Ts...>>
        requires assignable<T, Arg>
    constexpr variant& operator=( Arg&& arg ) noexcept( std::is_nothrow_constructible_v<T, Arg> && std::is_nothrow_assignable_v<T, Arg> ) {
        impl_.template assign<I>( wyne::forward<Arg>( arg ) );
        return *this;
    }

    template <std::size_t I, class... Args, constructible<Args...> T = type_pack_element_t<I, Ts...>>
    constexpr T& emplace( Args&&... args ) noexcept( std::is_nothrow_constructible_v<T, Args...> ) {
        return impl_.template emplace<I>( wyne::forward<Args>( args )... );
    }

    template <std::size_t I, class U, class... Args, constructible<std::initializer_list<U>, Args...> T = type_pack_element_t<I, Ts...>>
    constexpr T& emplace( std::initializer_list<U> il,
                          Args&&... args ) noexcept( std::is_nothrow_constructible_v<T, std::initializer_list<U>, Args...> ) {
        return impl_.template emplace<I>( il, wyne::forward<Args>( args )... );
    }

    constexpr bool valueless_by_exception() noexcept { return this->valueless_by_exception(); }

    constexpr std::size_t index() noexcept { return impl_.index(); }

    void swap( variant& that ) noexcept( wyne::all<( std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_swappable_v<Ts> )...> )
        requires( wyne::all<( std::is_move_constructible_v<Ts> && std::is_swappable_v<Ts> )...> )
    {
        impl_.swap( that.impl_ );
    }

private:
    core::impl<Ts...> impl_;
};

template <std::size_t I, class... Ts>
inline constexpr bool holds_alternative( const variant<Ts...>& v ) noexcept {
    return v.index() == I;
}

template <class T, class... Ts>
inline constexpr bool holds_alternative( const variant<Ts...>& v ) noexcept {
    return holds_alternative<core::find_index_checked_t<T, Ts...>>( v );
}

namespace core {

    template <std::size_t I, class V>
    struct generic_get_impl {
        constexpr generic_get_impl( int ) noexcept;

        constexpr WYNE_AUTO_FR operator()( V&& v ) const noexcept WYNE_AUTO_FR_RETURN( access::variant::get_alt<I>( wyne::forward<V>( v ) ).value );
    };

    template <std::size_t I, is_variant V>
    inline constexpr WYNE_AUTO_FR generic_get( V&& v ) noexcept
        WYNE_AUTO_FR_RETURN( generic_get_impl( holds_alternative<I>( v ) ? 0 : ( throw_bad_variant_access(), 0 ) )( wyne::forward<V>( v ) ) );

    template <std::size_t I, is_variant V>
    inline constexpr WYNE_AUTO generic_get_if( V* v ) noexcept
        WYNE_AUTO_RETURN( v&& holds_alternative<I>( *v ) ? std::addressof( access::variant::get_alt<I>( *v ).value ) : nullptr );

    inline constexpr bool is_any( std::initializer_list<bool> bl ) noexcept {
        for ( bool b : bl )
            if ( b )
                return true;
        return false;
    }

};  // namespace core

template <std::size_t I, is_variant V>
inline constexpr WYNE_AUTO_FR get( V&& v ) noexcept WYNE_AUTO_FR_RETURN( wyne::forward_like<V>( core::generic_get<I>( wyne::forward<V>( v ) ) ) );

template <class T, is_variant V>
inline constexpr WYNE_AUTO_FR get( V&& v ) noexcept
    WYNE_AUTO_FR_RETURN( wyne::get<unpack_to<std::remove_cvref_t<V>, core::find_index_checked, T>::value>( wyne::forward<V>( v ) ) );

template <std::size_t I, is_variant V>
inline constexpr WYNE_AUTO get_if( V* v ) noexcept WYNE_AUTO_RETURN( core::generic_get_if<I>( v ) );

template <class T, is_variant V>
inline constexpr WYNE_AUTO get_if( V* v ) noexcept
    WYNE_AUTO_RETURN( wyne::get_if<unpack_to<std::remove_cvref_t<V>, core::find_index_checked, T>::value>( v ) );

template <class... Ts>
inline constexpr auto operator<=>( const variant<Ts...>& lhs, const variant<Ts...>& rhs ) noexcept(
    ( noexcept( std::declval<std::compare_three_way>()( std::declval<Ts>(), std::declval<Ts>() ) ) && ... ) ) {
    std::size_t lhs_index_ = lhs.index(), rhs_index_ = rhs.index();
    if ( lhs_index_ != rhs_index_ )
        return lhs_index_ <=> rhs_index_;
    if ( lhs.valueless_by_exception() )
        return std::strong_ordering::equivalent;

    return core::visitation::variant::visit_alt_at(
        lhs.index(), []( auto&& lhs, auto&& rhs ) { return std::compare_three_way{}( lhs, rhs ); }, lhs, rhs );
}

struct monostate {};

inline constexpr bool operator<( monostate, monostate ) noexcept { return false; }

inline constexpr bool operator>( monostate, monostate ) noexcept { return false; }

inline constexpr bool operator<=( monostate, monostate ) noexcept { return true; }

inline constexpr bool operator>=( monostate, monostate ) noexcept { return true; }

inline constexpr bool operator==( monostate, monostate ) noexcept { return true; }

inline constexpr bool operator!=( monostate, monostate ) noexcept { return false; }

template <class Visitor, class... Vs>
inline constexpr WYNE_DECLTYPE_AUTO visit( Visitor&& visitor, Vs&&... vs )
    WYNE_DECLTYPE_AUTO_RETURN( ( !core::is_any( { vs.valueless_by_exception()... } ) ? ( void )0 : throw_bad_variant_access() ),
                               core::visitation::variant::visit_value( wyne::forward<Visitor>( visitor ), wyne::forward<Vs>( vs )... ) );

template <class... Ts>
inline constexpr void swap( const variant<Ts...>& lhs, const variant<Ts...>& rhs ) noexcept {
    return lhs.swap( rhs );
}

}  // namespace wyne

#endif