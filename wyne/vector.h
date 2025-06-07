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
#include "assert.h"
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
    using size_type              = typename std::allocator_traits<allocator_type>::size_type;
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

    constexpr allocator_type&       alloc() noexcept { return end_cap_.second(); }
    constexpr const allocator_type& alloc() const noexcept { return end_cap_.second(); }
    constexpr pointer&              end_cap() noexcept { return end_cap_.first(); }
    constexpr const pointer&        end_cap() const noexcept { return end_cap_.first(); }

    constexpr iterator make_iter( pointer p ) noexcept { return iterator( p ); }

    constexpr const_iterator make_iter( const_pointer p ) noexcept { return const_iterator( p ); }

    void throw_length_error() const { throw std::length_error( "vector" ); }

    void throw_out_of_range() const { throw std::out_of_range( "vector" ); }

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

    const void vdeallocate() noexcept {
        if ( begin_ != nullptr ) {
            clear();
            alloc_traits::deallocate( alloc(), begin_, capacity() );
            begin_ = end_ = end_cap() = nullptr;
        }
    }

    template <class... Args>
    constexpr void construct( pointer p, Args... args ) {
        alloc_traits::construct( alloc(), std::to_address( p ), wyne::forward<Args>( args )... );
    }

    constexpr void destroy( pointer p ) noexcept { alloc_traits::destroy( alloc(), std::to_address( p ) ); }

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

    constexpr void destruct_at_end( pointer new_last ) noexcept {
        pointer pos = end_;
        for ( ; pos != new_last; --pos )
            destroy( pos );
        end_ = new_last;
    }

    inline constexpr size_type recommend( size_type new_size ) const {
        const size_type ms = max_size();
        if WYNE_UNLIKELY ( new_size > ms )
            throw_length_error();
        const size_type cap = capacity();
        if WYNE_UNLIKELY ( cap >= ( ms >> 1 ) )
            return ms;
        return wyne::max<size_type>( cap << 1, new_size );
    }

    constexpr void swap_out_circular_buffer( split_buffer<value_type, allocator_type&>& v_ ) {
        wyne::uninitialized_allocator_relocate( alloc(), std::to_address( begin_ ), std::to_address( end_ ),
                                                std::to_address( v_.first_ ) );
        // assert( v_.first_ == ( v_.begin_ - ( end_ - begin_ ) ) );
        end_      = begin_;
        v_.begin_ = v_.first_;
        wyne::swap( this->begin_, v_.begin_ );
        wyne::swap( this->end_, v_.end_ );
        wyne::swap( this->end_cap(), v_.end_cap() );
        v_.first_ = v_.begin_;
    }

    template <class InputIterator>
    constexpr void init_with_sentienl( InputIterator first, InputIterator last ) {
        for ( ; first != last; ++first )
            emplace_back( *first );
    }

    template <class ForwardIterator>
    constexpr void init_with_size( ForwardIterator first, ForwardIterator last, size_type n ) {
        if WYNE_LIKELY ( n > 0 ) {
            vallocate( n );
            construct_at_end( first, last, n );
        }
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

    template <
        class InputIterator,
        std::enable_if_t<is_exactly_input_iterator_t<InputIterator>
                             && std::is_constructible_v<value_type, typename iterator_traits<InputIterator>::reference>,
                         int> = 0>
    constexpr vector( InputIterator first, InputIterator last ) {
        init_with_sentienl( first, last );
    }

    template <
        class InputIterator,
        std::enable_if_t<is_exactly_input_iterator_t<InputIterator>
                             && std::is_constructible_v<value_type, typename iterator_traits<InputIterator>::reference>,
                         int> = 0>
    constexpr vector( InputIterator first, InputIterator last, const allocator_type& a ) : end_cap_( nullptr, a ) {
        init_with_sentienl( first, last );
    }

    template <class ForwardIterator,
              std::enable_if_t<
                  is_forward_iterator_t<ForwardIterator>
                      && std::is_constructible_v<value_type, typename iterator_traits<ForwardIterator>::value_type>,
                  int> = 0>
    constexpr vector( ForwardIterator first, ForwardIterator last ) {
        size_type n = static_cast<size_type>( wyne::distance( first, last ) );
        init_with_size( first, last, n );
    }

    template <class ForwardIterator,
              std::enable_if_t<
                  is_forward_iterator_t<ForwardIterator>
                      && std::is_constructible_v<value_type, typename iterator_traits<ForwardIterator>::value_type>,
                  int> = 0>
    constexpr vector( ForwardIterator first, ForwardIterator last, const allocator_type& a ) : end_cap_( nullptr, a ) {
        size_type n = static_cast<size_type>( wyne::distance( first, last ) );
        init_with_size( first, last, n );
    }

    constexpr vector( const vector& x )
        : end_cap_( nullptr, alloc_traits::select_on_container_copy_construction( x.alloc() ) ) {
        init_with_size( x.begin_, x.end_, x.size() );
    }

    constexpr vector( vector&& x ) : end_cap_( nullptr, wyne::move( x.alloc() ) ) {
        begin_     = x.begin_;
        end_       = x.end_;
        end_cap_() = x.end_cap();
        x.begin_ = x.end_ = x.end_cap() = nullptr;
    }

    constexpr ~vector() noexcept {
        if ( begin_ != nullptr ) {
            clear();
            alloc_traits::deallocate( alloc(), begin_, capacity() );
        }
    }

    constexpr size_type max_size() const noexcept {
        return wyne::min<size_type>( alloc_traits::max_size( alloc() ), std::numeric_limits<difference_type>::max() );
    }

    constexpr iterator       begin() noexcept { return make_iter( begin_ ); }
    constexpr const_iterator begin() const noexcept { return begin(); }
    constexpr iterator       end() noexcept { return make_iter( end_ ); }
    constexpr const_iterator end() const noexcept { return end(); }

    constexpr reverse_iterator       rbegin() noexcept { return reverse_iterator( end() ); }
    constexpr const_reverse_iterator rbegin() const noexcept { return rbegin(); }
    constexpr reverse_iterator       rend() noexcept { return reverse_iterator( begin() ); }
    constexpr const_reverse_iterator rend() const noexcept { return rend(); }

    constexpr const_iterator         cbegin() noexcept { return begin(); }
    constexpr const_iterator         cend() noexcept { return end(); }
    constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    constexpr const_reverse_iterator crend() const noexcept { return rend(); }

    constexpr size_type capacity() const noexcept { return static_cast<size_type>( end_cap() - begin_ ); }
    constexpr size_type size() const noexcept { return static_cast<size_type>( end_ - begin_ ); }
    constexpr bool      empty() const noexcept { return end_ == begin_; }

    constexpr void clear() noexcept { destruct_at_end( begin_ ); }

    constexpr void reserve( size_type n ) {
        if ( n > capacity() ) {
            if ( n > max_size() )
                throw_length_error();
            split_buffer<value_type, allocator_type&> v( n, size(), alloc() );
            swap_out_circular_buffer( v );
        }
    }

    constexpr void shrink_to_fit() noexcept {
        if ( capacity() > size() ) {
            try {
                split_buffer<value_type, allocator_type&> v( size(), size(), alloc() );
                swap_out_circular_buffer( v );
            }
            catch ( ... ) {
            }
        }
    }

    constexpr reference operator[]( size_type n ) noexcept {
        WYNE_ASSERT_VALID_ELEMENT_ACCESS( n < size(), "message" );
        return begin_[ n ];
    }

    constexpr const_reference operator[]( size_type n ) const noexcept {
        WYNE_ASSERT_VALID_ELEMENT_ACCESS( n < size(), "message" );
        return begin_[ n ];
    }

    constexpr reference at( size_type n ) {
        if ( n > size() )
            throw_out_of_range();
        return begin_[ n ];
    }

    constexpr const_reference at( size_type n ) const {
        if ( n > size() )
            throw_out_of_range();
        return begin_[ n ];
    }

    constexpr reference front() noexcept {
        WYNE_ASSERT_VALID_ELEMENT_ACCESS( !empty(), "front() called on an empty vector" );
        return *begin_;
    }

    constexpr const_reference front() const noexcept {
        WYNE_ASSERT_VALID_ELEMENT_ACCESS( !empty(), "front() called on an empty vector" );
        return *begin_;
    }

    constexpr reference back() noexcept {
        WYNE_ASSERT_VALID_ELEMENT_ACCESS( !empty(), "back() called on an empty vector" );
        return *( end_ - 1 );
    }

    constexpr const_reference back() const noexcept {
        WYNE_ASSERT_VALID_ELEMENT_ACCESS( !empty(), "back() called on an empty vector" );
        return *( end_ - 1 );
    }

    constexpr value_type* data() noexcept { return std::to_address( begin_ ); }

    constexpr value_type* data() const noexcept { return std::to_address( begin_ ); }

    constexpr void push_back( const_reference x ) { emplace_back( x ); }

    constexpr void push_back( value_type&& x ) { emplace_back( wyne::move( x ) ); }

    template <class... Args>
    constexpr pointer emplace_back_slow_path( Args&&... args ) {
        const size_type                           sz = size();
        split_buffer<value_type, allocator_type&> v_( recommend( sz + 1 ), sz, alloc() );
        construct( v_.end_, wyne::forward<Args>( args )... );
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

    constexpr void pop_back() noexcept {
        WYNE_ASSERT_VALID_ELEMENT_ACCESS( !empty(), "pop_back() called on an empty vector" );
        destruct_at_end( end_ - 1 );
    }

    template <class InputIterator,
              std::enable_if_t<
                  is_exactly_input_iterator_t<InputIterator>
                      && std::is_constructible_v<value_type, typename iterator_traits<InputIterator>::value_type>,
                  int> = 0>
    constexpr void assign( InputIterator first, InputIterator last ) {
        clear();
        for ( ; first != last; ++first )
            emplace_back( *first );
    }

    template <class ForwardIterator,
              std::enable_if_t<
                  is_forward_iterator_t<ForwardIterator>
                      && std::is_constructible_v<value_type, typename iterator_traits<ForwardIterator>::value_type>,
                  int> = 0>
    constexpr void assign( ForwardIterator first, ForwardIterator last ) {
        const size_type new_size = static_cast<size_type>( wyne::distance( first, last ) );
        if ( new_size <= capacity() ) {
            if ( new_size <= size() ) {
                pointer m = wyne::copy( first, last, begin_ );
                destruct_at_end( m );
            }
            else {
                const auto mid = wyne::advance( first, size() );
                wyne::copy( first, mid, begin_ );
                construct_at_end( mid, last, new_size - size() );
            }
        }
        else {
            vdeallocate();
            vallocate( recommend( new_size ) );
            construct_at_end( first, last, new_size );
        }
    }

    constexpr void assign( size_type n, const_reference x ) {
        if ( n <= capacity() ) {
            size_type s = size();
            wyne::fill_n( begin_, wyne::min( n, s ), x );
            if ( n <= s )
                destruct_at_end( begin_ + n );
            else
                construct_at_end( n - s, x );
        }
        else {
            vdeallocate();
            vallocate( recommend( n ) );
            construct_at_end( n, x );
        }
    }
};

};  // namespace wyne

#endif