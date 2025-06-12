#ifndef WYNE_ALLOCATOR_H__
#define WYNE_ALLOCATOR_H__

#include <cstddef>
#include <new>

#include "config.h"
#include "construct.h"
#include "util.h"

namespace wyne {

template <class Tp>
class allocator {
public:
    using value_type      = Tp;
    using pointer         = Tp*;
    using const_pointer   = const Tp*;
    using reference       = Tp&;
    using const_reference = const Tp&;
    using size_type       = size_t;
    using difference_type = ptrdiff_t;

public:
    static constexpr Tp* allocate();
    static constexpr Tp* allocate( size_type n );

    static constexpr void deallocate( Tp* ptr ) noexcept;
    static constexpr void deallocate( Tp* ptr, size_type n ) noexcept;

    template <class... Args>
    static constexpr void construct( Tp* ptr, Args&&... args );

    static constexpr void destroy( Tp* ptr ) noexcept;
    static constexpr void destroy( Tp* first, Tp* last );
};

template <class Tp>
constexpr Tp* allocator<Tp>::allocate() {
    if WYNE_LIKELY ( wyne::requires_special_alignment<Tp>() ) {
        return static_cast<Tp*>( ::operator new( sizeof( Tp ), static_cast<std::align_val_t>( alignof( Tp ) ) ) );
    }
    return static_cast<Tp*>( ::operator new( sizeof( Tp ) ) );
}

template <class Tp>
constexpr Tp* allocator<Tp>::allocate( size_type n ) {
    if WYNE_LIKELY ( wyne::requires_special_alignment<Tp>() ) {
        return static_cast<Tp*>( ::operator new( n * sizeof( Tp ), static_cast<std::align_val_t>( alignof( Tp ) ) ) );
    }
    return static_cast<Tp*>( ::operator new( n * sizeof( Tp ) ) );
}

template <class Tp>
constexpr void allocator<Tp>::deallocate( Tp* ptr ) noexcept {
    if WYNE_LIKELY ( wyne::requires_special_alignment<Tp>() ) {
        ::operator delete( ptr, static_cast<std::align_val_t>( alignof( Tp ) ) );
        return;
    }
    ::operator delete( ptr );
}

template <class Tp>
constexpr void allocator<Tp>::deallocate( Tp* ptr, size_type n ) noexcept {
    allocator::deallocate( ptr );
}

template <class Tp>
template <class... Args>
constexpr void allocator<Tp>::construct( Tp* ptr, Args&&... args ) {
    wyne::construct_at( ptr, wyne::forward<Args>( args )... );
}

template <class Tp>
constexpr void allocator<Tp>::destroy( Tp* ptr ) noexcept {
    wyne::destroy_at( ptr );
}

template <class Tp>
constexpr void allocator<Tp>::destroy( Tp* first, Tp* last ) {
    wyne::destroy( first, last );
}

template <class _Tp, class _Up>
inline constexpr bool operator==( const allocator<_Tp>&, const allocator<_Up>& ) _NOEXCEPT {
    return true;
}

};  // namespace wyne

#endif