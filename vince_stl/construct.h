#ifndef VINCE_CONSTRUCT_H__
#define VINCE_CONSTRUCT_H__

#include <memory>
#include <utility>

namespace vince {

// construct

template <class Tp, class... Args>
void construct_at( const Tp* p, Args&&... args ) {
    ::new ( static_cast<void*>( p ) ) Tp( std::forward<Args>( args )... );
}

// destroy

template <class Tp>
void __destroy_at( Tp* p ) {
    p->~Tp();
}

template <class ForwardIterator>
ForwardIterator __destroy( ForwardIterator first, ForwardIterator last ) {
    for ( ; first != last; ++first )
        vince::__destroy_at( &*first );
    return first;
}

template <class ForwardIterator>
ForwardIterator __reverse_destroy( ForwardIterator first, ForwardIterator last ) {
    while ( last != first ) {
        --last;
        vince::__destroy_at( &*last );
    }
    return last;
}

template <class Tp>
void destroy_at( Tp* p ) {
    vince::__destroy_at( p );
}

template <class ForwardIterator>
void destroy( ForwardIterator first, ForwardIterator last ) {
    ( void )vince::__destroy( std::move( first ), std::move( last ) );
}

template <class ForwardIterator, class Size>
ForwardIterator destroy_n( ForwardIterator first, Size n ) {
    for ( ; n > 0; ( void )++first, --n )
        vince::__destroy_at( &*first );
    return first;
}

}  // namespace vince

#endif