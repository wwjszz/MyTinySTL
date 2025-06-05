#ifndef WYNE_VECTOR_H__
#define WYNE_VECTOR_H__

#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>

#include "algobase.h"
#include "allocator.h"
#include "compressed_pair.h"
#include "config.h"
#include "iterator.h"
#include "split_buffer.h"
#include "uninitialized.h"
#include "util.h"

namespace wyne {

template <class Tp, class Allocator = wyne::allocator<Tp>>
class vector {
    static_assert( std::is_same_v<typename Allocator::value_type, Tp>, "" );

public:
    using value_type             = Tp;
    using allocator_type         = Allocator;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using pointer                = typename std::allocator_traits<allocator_type>::pointer;
    using const_pointer          = typename std::allocator_traits<allocator_type>::const_pointer;
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = wyne::reverse_iterator<iterator>;
    using const_reverse_iterator = wyne::reverse_iterator<const_iterator>;

private:
    using alloc_traits = std::allocator_traits<allocator_type>;

    pointer                                  begin_ = nullptr;
    pointer                                  end_   = nullptr;
    compressed_pair<pointer, allocator_type> end_cap_ =
        compressed_pair<pointer, allocator_type>( nullptr, default_init_tag() );

    constexpr allocator_type& alloc() noexcept { return end_cap_.second(); }

    constexpr const allocator_type& alloc() const noexcept { return end_cap_.second(); }

    constexpr pointer& end_cap() noexcept { return end_cap_.first(); }

    constexpr const pointer& end_cap() const noexcept { return end_cap_.first(); }

    void throw_length_error() const { throw std::length_error( "vector" ); }

    struct ConstructTransaction {
        constexpr explicit ConstructTransaction( vector& v, size_type n ) noexcept
            : v( v ), pos( v.end_ ), new_end( v.end_ + n ) {}

        constexpr ~ConstructTransaction() noexcept { v.end_ = pos; }

        vector&             v;
        pointer             pos;
        const_pointer const new_end;

        ConstructTransaction( ConstructTransaction const& )            = delete;
        ConstructTransaction& operator=( ConstructTransaction const& ) = delete;
    };

    constexpr void vallocate( size_type n ) {
        if WYNE_UNLIKELY ( n > max_size() )
            throw_length_error();
        begin_    = alloc_traits::allocate( alloc(), n );
        end_      = begin_;
        end_cap() = begin_ + n;
    }

    template <class... Args>
    constexpr void construct( pointer p, Args... args ) {
        alloc_traits::construct( alloc(), std::to_address( p ), wyne::forward<Args>( args )... );
    }

    template <class... Args>
    constexpr void construct_one_at_end( Args&&... args ) {
        ConstructTransaction ct( *this, 1 );
        construct( ct.pos++, wyne::forward<Args>( args )... );
    }

    constexpr void construct_at_end( size_type n ) {
        ConstructTransaction ct( *this, n );
        const_pointer const  new_end = ct.new_end;
        for ( ; ct.pos != new_end; ++ct.pos ) {
            alloc_traits::construct( alloc(), std::to_address( ct.pos ) );
        }
    }

    constexpr void construct_at_end( size_type n, const_reference x ) {
        ConstructTransaction ct( *this, n );
        const_pointer const  new_end = ct.new_end;
        for ( ; ct.pos != new_end; ++ct.pos ) {
            alloc_traits::construct( alloc(), std::to_address( ct.pos ), x );
        }
    }

    template <class InputIterator>
    constexpr void construct_at_end( InputIterator first, InputIterator last, size_type n ) {
        ConstructTransaction ct( *this, n );
        ct.pos = wyne::uninitialized_allocator_copy( alloc(), first, last, ct.pos );
    }

    inline constexpr size_type recommend( size_type new_size ) const {
        const size_type ms = max_size();
        if ( new_size > ms )
            throw_length_error();
        const size_type cap = capacity();
        if ( cap >= ( ms >> 1 ) )
            return ms;
        return wyne::max<size_type>( cap << 1, new_size );
    }

    constexpr void swap_out_circular_buffer( split_buffer<value_type, allocator_type&>& v_ ) {
        wyne::uninitialized_allocator_relocate( alloc(), std::to_address( begin_ ), std::to_address( end_ ),
                                                std::to_address( v_.first_ ) );
        end_      = begin_;
        v_.begin_ = v_.first_;
        wyne::swap( this->begin_, v_.begin_ );
        wyne::swap( this->end_, v_.end_ );
        wyne::swap( this->end_cap(), v_.end_cap() );
    }

public:
    constexpr vector() noexcept( std::is_nothrow_default_constructible_v<allocator_type> ) {}

    constexpr explicit vector( const allocator_type& a ) noexcept : end_cap_( nullptr, a ) {}

    constexpr explicit vector( size_type n ) {
        if WYNE_LIKELY ( n > 0 ) {
            vallocate( n );
            construct_at_end( n );
        }
    }

    constexpr explicit vector( size_type n, const allocator_type& a ) : end_cap_( nullptr, a ), vector( n ) {}

    constexpr vector( size_type n, const value_type& x ) {
        if WYNE_LIKELY ( n > 0 ) {
            vallocate( n );
            construct_at_end( n, x );
        }
    }

    constexpr vector( size_type n, const value_type& x, const allocator_type& a )
        : end_cap_( nullptr, a ), vector( n, x ) {}

    constexpr size_type max_size() const noexcept {
        return wyne::min<size_type>( alloc_traits::max_size( alloc() ), std::numeric_limits<difference_type>::max() );
    }

    constexpr size_type capacity() const noexcept { return static_cast<size_type>( end_cap() - begin_ ); }

    constexpr size_type size() const noexcept { return static_cast<size_type>( end_ - begin_ ); }

    constexpr bool empty() const noexcept { return end_ == begin_; }

    template <class... Args>
    inline constexpr pointer emplace_back_slow_path( Args&&... args ) {
        const size_type                           sz = size();
        split_buffer<value_type, allocator_type&> v_( recommend( sz + 1 ), sz, alloc() );
        construct( v_.begin_, wyne::forward<Args>( args )... );
        ++v_.end_;
        swap_out_circular_buffer( v_ );
        return this->end_;
    }

    template <class... Args>
    inline constexpr reference emplace_back( Args&&... args ) {
        auto end_ = this->end_;
        if ( end_ < this->end_cap() ) {
            construct_one_at_end( wyne::forward<Args>( args )... );
            ++end_;
        }
        else {
            end_ = emplace_back_slow_path( wyne::forward<Args>( args )... );
        }
        // assert( this->end_ == end_ );
        // this->end_ = end_;
        return *( end_ - 1 );
    }
};

};  // namespace wyne

#endif