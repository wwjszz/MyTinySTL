#ifndef VINCE_ALGOBASE_H__
#define VINCE_ALGOBASE_H__

#include "util.h"

#define STRINGIFY( x ) #x
#define COMPILER_MESSAGE( msg ) _Pragma( STRINGIFY( GCC warning msg ) )

#ifdef max
COMPILER_MESSAGE( "#undefing macro max" )
#undef max
#endif

#ifdef min
COMPILER_MESSAGE( "#undefing macro min" )
#undef min
#endif

namespace vince {

template <class T>
inline const T& max( const T& a, const T& b ) {
    return a > b ? a : b;
}

template <class T>
inline const T& min( const T& a, const T& b ) {
    return a < b ? a : b;
}

template <class T, class Comp>
inline const T& max( const T& a, const T& b, Comp comp ) {
    return comp( b, a ) ? a : b;
}

template <class T, class Comp>
inline const T& min( const T& a, const T& b, Comp comp ) {
    return comp( a, b ) ? a : b;
}

template <class ForwardIterator1, class ForwardIterator2>
inline void iter_swap( ForwardIterator1 a,
                       ForwardIterator2 b ) noexcept( noexcept( swap( *std::declval<ForwardIterator1>(),
                                                                      *std::declval<ForwardIterator2>() ) ) ) {
    swap( *a, *b );
}

}  // namespace vince

#endif