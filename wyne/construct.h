#ifndef WYNE_CONSTRUCT_H__
#define WYNE_CONSTRUCT_H__

#include "util.h"

namespace wyne {

// construct

template <class Tp, class... Args>
constexpr void construct_at( Tp* p, Args&&... args ) {
    ::new ( static_cast<void*>( p ) ) Tp( std::forward<Args>( args )... );
}

// destroy

template <class Tp>
constexpr void __destroy_at( Tp* p ) noexcept {
    p->~Tp();
}

template <class ForwardIterator>
constexpr ForwardIterator __destroy( ForwardIterator first, ForwardIterator last ) {
    for ( ; first != last; ++first )
        wyne::__destroy_at( &*first );
    return first;
}

template <class ForwardIterator>
constexpr ForwardIterator __reverse_destroy( ForwardIterator first, ForwardIterator last ) {
    while ( last != first ) {
        --last;
        wyne::__destroy_at( &*last );
    }
    return last;
}

template <class Tp>
constexpr void destroy_at( Tp* p ) noexcept {
    wyne::__destroy_at( p );
}

template <class ForwardIterator>
constexpr void destroy( ForwardIterator first, ForwardIterator last ) noexcept {
    ( void )wyne::__destroy( wyne::move( first ), wyne::move( last ) );
}

template <class ForwardIterator, class Size>
constexpr ForwardIterator destroy_n( ForwardIterator first, Size n ) {
    for ( ; n > 0; ( void )++first, --n )
        wyne::__destroy_at( &*first );
    return first;
}

}  // namespace wyne

#endif