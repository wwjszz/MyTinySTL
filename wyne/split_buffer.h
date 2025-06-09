#ifndef WYNE_SPLIT_BUFFER_H__
#define WYNE_SPLIT_BUFFER_H__

#include <memory>
#include <type_traits>

#include "allocator.h"
#include "compressed_pair.h"
#include "memory/swap_allocator.h"
#include "uninitialized.h"
#include "util.h"

namespace wyne {

// Currently only used for vector. It may be completed in the future.

template <class Tp, class Allocator = allocator<Tp>>
class split_buffer {
    static_assert( std::is_lvalue_reference<Allocator>::value, "Allocator must be an lvalue reference as only being used by vector." );

public:
    using value_type      = Tp;
    using allocator_type  = Allocator;
    using alloc_rr        = std::remove_reference_t<allocator_type>;
    using alloc_traits    = std::allocator_traits<alloc_rr>;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using size_type       = typename alloc_traits::size_type;
    using difference_type = typename alloc_traits::difference_type;
    using pointer         = typename alloc_traits::pointer;
    using const_pointer   = typename alloc_traits::const_pointer;
    using iterator        = pointer;
    using const_iterator  = const_pointer;

    pointer                            first_;
    pointer                            begin_;
    pointer                            end_;
    compressed_pair<pointer, alloc_rr> end_cap_;

    constexpr split_buffer( size_type cap, size_type start, alloc_rr& a ) : end_cap_( nullptr, a ) {
        if WYNE_UNLIKELY ( cap == 0 ) {
            first_ = nullptr;
        }
        else {
            first_ = alloc_traits::allocate( alloc(), cap );
        }
        begin_ = end_ = first_ + start;
        end_cap()     = first_ + cap;
    }

    constexpr ~split_buffer() {
        clear();
        alloc_traits::deallocate( alloc(), first_, capacity() );
    }

    constexpr alloc_rr& alloc() noexcept { return end_cap_.second(); }

    constexpr const alloc_rr& alloc() const noexcept { return end_cap_.second(); }

    constexpr pointer& end_cap() noexcept { return end_cap_.first(); }

    constexpr const pointer& end_cap() const noexcept { return end_cap_.first(); }

    const size_type size() const noexcept { return static_cast<size_type>( end_ - begin_ ); }

    const bool empty() const noexcept { return end_ == begin_; }

    const size_type front_spare() const noexcept { return static_cast<size_type>( begin_ - first_ ); }

    const size_type back_spare() const noexcept { return static_cast<size_type>( end_cap() - end_ ); }

    const size_type capacity() const noexcept { return static_cast<size_type>( end_cap() - first_ ); }

    constexpr void clear() {
        for ( ; begin_ != end_; ++begin_ )
            alloc_traits::destroy( alloc(), std::to_address( begin_ ) );
    }

    // NOTE: split_buffer is currently only used as the underlying storage for vector.
    // Therefore, it only provides unchecked emplace operations at the front and back.
    // These operations assume that there is sufficient space available and do not perform safety checks.
    template <class... Args>
    constexpr void unsafe_emplace_back( Args&&... args ) {
        construct( end_, wyne::forward<Args>( args )... );
        ++end_;
    }

    template <class... Args>
    constexpr void unsafe_emplace_front( Args&&... args ) {
        construct( begin_ - 1, wyne::forward<Args>( args )... );
        --begin_;
    }

    constexpr void push_back( const_reference x ) { unsafe_emplace_back( x ); }

    constexpr void push_back( value_type&& x ) { unsafe_emplace_back( wyne::move( x ) ); }

    split_buffer( const split_buffer& )            = delete;
    split_buffer& operator=( const split_buffer& ) = delete;

    template <class... Args>
    constexpr void construct( pointer p, Args&&... args ) {
        alloc_traits::construct( alloc(), std::to_address( p ), wyne::forward<Args>( args )... );
    }

    constexpr void construct_at_end( size_type n ) {
        ConstructTransaction ct( &this->end_, n );
        for ( ; ct.pos != ct.end; ++ct.pos )
            construct( ct.pos );
    }

    constexpr void construct_at_end( size_type n, const_reference x ) {
        ConstructTransaction ct( &this->end_, n );
        for ( ; ct.pos != ct.end; ++ct.pos )
            construct( ct.pos, x );
    }

    template <class InputIterator>
    constexpr void construct_at_end( InputIterator first, InputIterator last ) {
        // TODO: use ConstructTrainsaction
        end_ = wyne::uninitialized_allocator_copy( alloc(), first, last, end_ );
    }

    template <class ForwardIterator>
    constexpr void construct_at_end_with_size( ForwardIterator first, size_type n ) {
        // TODO: use ConstructTrainsaction
        end_ = wyne::uninitialized_allocator_copy_n( alloc(), first_, n, end_ );
    }

    template <class InputIterator>
    constexpr void construct_at_end_with_sentinel( InputIterator first, InputIterator last ) {
        for ( ; first != last; ++first ) {
            if ( end_ == end_cap() ) {
                size_type    old_cap = end_cap() - first;
                size_type    new_cap = wyne::max<size_type>( old_cap << 1, 8 );
                split_buffer buf( new_cap, 0, alloc() );
                for ( pointer p = begin_; p != end_; ++p )
                    buf.construct_at_end( wyne::move( *p ) );

                swap( buf );
            }
            construct( end_, *first );
            ++end_;
        }
    }

    struct ConstructTransaction {
        constexpr explicit ConstructTransaction( pointer* p, size_type n ) noexcept : pos( *p ), end( *p + n ), dest( p ) {}

        constexpr ~ConstructTransaction() noexcept { *dest = pos; }
        pointer       pos;
        pointer const end;

        ConstructTransaction( ConstructTransaction const& )            = delete;
        ConstructTransaction& operator=( ConstructTransaction const& ) = delete;

    private:
        pointer* dest;
    };

    constexpr void swap( split_buffer& x ) noexcept {
        wyne::swap( begin_, x.begin_ );
        wyne::swap( end_, x.end_ );
        wyne::swap( end_cap(), x.end_cap() );
        wyne::swap_allocator( alloc(), x.alloc() );
    }
};

}  // namespace wyne

#endif