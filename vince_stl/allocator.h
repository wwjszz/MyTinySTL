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
    typedef Tp        value_type;
    typedef Tp*       pointer;
    typedef const Tp* const_pointer;
    typedef Tp&       reference;
    typedef const Tp& const_reference;
    typedef size_t    size_type;
    typedef ptrdiff_t difference_type;

public:
    static Tp* allocate();
    static Tp* allocate( size_type n );

    static void deallocate( Tp* ptr ) noexcept;
    static void deallocate( Tp* ptr, size_type n ) noexcept;

    template <class... Args>
    static void construct( Tp* ptr, Args&&... args );

    static void destory( Tp* ptr ) noexcept;
    static void destory( Tp* first, Tp* last );
};

template <class Tp>
Tp* allocator<Tp>::allocate() {
    if WYNE_LIKELY ( wyne::requires_special_alignment<Tp>() ) {
        return static_cast<Tp*>( ::operator new( sizeof( Tp ), static_cast<std::align_val_t>( alignof( Tp ) ) ) );
    }
    return static_cast<Tp*>( ::operator new( sizeof( Tp ) ) );
}

template <class Tp>
Tp* allocator<Tp>::allocate( size_type n ) {
    if WYNE_LIKELY ( wyne::requires_special_alignment<Tp>() ) {
        return static_cast<Tp*>( ::operator new( n * sizeof( Tp ), static_cast<std::align_val_t>( alignof( Tp ) ) ) );
    }
    return static_cast<Tp*>( ::operator new( n * sizeof( Tp ) ) );
}

template <class Tp>
void allocator<Tp>::deallocate( Tp* ptr ) noexcept {
    if WYNE_LIKELY ( wyne::requires_special_alignment<Tp>() ) {
        ::operator delete( ptr, static_cast<std::align_val_t>( alignof( Tp ) ) );
        return;
    }
    ::operator delete( ptr );
}

template <class Tp>
void allocator<Tp>::deallocate( Tp* ptr, size_type n ) noexcept {
    allocator::deallocate( ptr );
}

template <class Tp>
template <class... Args>
void allocator<Tp>::construct( Tp* ptr, Args&&... args ) {
    wyne::construct_at( ptr, wyne::forward<Args>( args )... );
}

template <class Tp>
void allocator<Tp>::destory( Tp* ptr ) noexcept {
    wyne::destroy_at( ptr );
}

template <class Tp>
void allocator<Tp>::destory( Tp* first, Tp* last ) {
    wyne::destroy( first, last );
}

};  // namespace wyne

#endif