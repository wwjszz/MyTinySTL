#ifndef WYNE_VECTOR_H__
#define WYNE_VECTOR_H__

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <type_traits>

#include "algobase.h"
#include "allocator.h"
#include "assert.h"
#include "compressed_pair.h"
#include "config.h"
#include "iterator.h"
#include "split_buffer.h"
#include "type_traits.h"
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

    pointer                                  begin_   = nullptr;
    pointer                                  end_     = nullptr;
    compressed_pair<pointer, allocator_type> end_cap_ = compressed_pair<pointer, allocator_type>( nullptr, default_init_tag() );

    constexpr allocator_type&       alloc() noexcept { return end_cap_.second(); }
    constexpr const allocator_type& alloc() const noexcept { return end_cap_.second(); }
    constexpr pointer&              end_cap() noexcept { return end_cap_.first(); }
    constexpr const pointer&        end_cap() const noexcept { return end_cap_.first(); }

    constexpr iterator make_iter( pointer p ) noexcept { return iterator( p ); }

    constexpr const_iterator make_iter( const_pointer p ) noexcept { return const_iterator( p ); }

    void throw_length_error() const { throw std::length_error( "vector" ); }

    void throw_out_of_range() const { throw std::out_of_range( "vector" ); }

    struct ConstructTransaction {
        constexpr explicit ConstructTransaction( vector& v, size_type n ) noexcept : v( v ), pos( v.end_ ), new_end( v.end_ + n ) {}

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

    void vdeallocate() noexcept {
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
        wyne::uninitialized_allocator_relocate( alloc(), std::to_address( begin_ ), std::to_address( end_ ), std::to_address( v_.first_ ) );
        // assert( v_.first_ == ( v_.begin_ - ( end_ - begin_ ) ) );
        end_      = begin_;
        v_.begin_ = v_.first_;
        wyne::swap( this->begin_, v_.begin_ );
        wyne::swap( this->end_, v_.end_ );
        wyne::swap( this->end_cap(), v_.end_cap() );
        v_.first_ = v_.begin_;
    }

    constexpr void swap_out_circular_buffer( split_buffer<value_type, allocator_type&>& v, pointer p ) {
        pointer ret = v.begin_;

        // relocate [p, end_)
        wyne::uninitialized_allocator_relocate( alloc(), std::to_address( p ), std::to_address( end_ ), std::to_address( v.end_ ) );
        v.end_ += ( end_ - p );
        end_           = p;
        auto new_begin = v.begin_ - ( p - begin_ );

        // relocate [begin, p)
        wyne::uninitialized_allocator_relocate( alloc(), std::to_address( begin_ ), std::to_address( p ), std::to_address( new_begin ) );
        v.begin_ = new_begin;
        end_     = begin_;

        wyne::swap( begin_, v.begin_ );
        wyne::swap( end_, v.end_ );
        wyne::swap( end_cap_, v.end_cap() );
        v.first_ = v.begin_;
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

    constexpr void append( size_type n ) {
        if ( static_cast<size_type>( end_cap() - end_ ) >= n )
            construct_at_end( n );
        else {
            split_buffer<value_type, allocator_type&> v( recommend( size() + n ), size(), alloc() );
            v.construct_at_end( n );
            swap_out_circular_buffer( v );
        }
    }

    constexpr void append( size_type n, const_reference x ) {
        if ( static_cast<size_type>( end_cap() - end_ ) >= n )
            construct_at_end( n, x );
        else {
            split_buffer<value_type, allocator_type&> v( recommend( size() + n ), size(), alloc() );
            v.construct_at_end( n, x );
            swap_out_circular_buffer( v );
        }
    }

    constexpr void copy_assign_alloc( const vector& x ) {
        copy_assign_alloc( x, bool_constant<alloc_traits::propagate_on_container_copy_assignment::value>() );
    }

    constexpr void move_assign_alloc( vector&& x ) noexcept( !alloc_traits::propagate_on_container_move_assignment::value
                                                             || std::is_nothrow_move_assignable_v<allocator_type> ) {
        move_assign_alloc( x, bool_constant<alloc_traits::propagate_on_container_move_assignment::value>() );
    }

    constexpr void copy_assign_alloc( const vector& x, true_type ) {
        if ( alloc() != x.alloc() ) {
            vdeallocate();
            // clear();
            // alloc_traits::deallocate( alloc(), begin_, capacity() );
            // begin_ = end_ = end_cap() = nullptr;
        }
        alloc() = x.alloc();
    }

    constexpr void copy_assign_alloc( const vector& x, false_type ) {}

    constexpr void move_assign_alloc( vector&& x, true_type ) noexcept( std::is_nothrow_move_assignable_v<allocator_type> ) {
        alloc() = wyne::move( x.alloc() );
    }

    constexpr void move_assign_alloc( vector&& x, false_type ) noexcept {}

    constexpr void move_assign( vector&& x, true_type ) noexcept( std::is_nothrow_move_assignable_v<allocator_type> ) {
        vdeallocate();
        move_assign_alloc( x );  // this can throw
        begin_    = x.begin_;
        end_      = x.end_;
        end_cap() = x.end_cap();
        x.begin_ = x.end_ = x.end_cap() = nullptr;
    }

    constexpr void move_assign( vector&& x, false_type ) noexcept( alloc_traits::is_always_equal::value ) {
        if ( alloc() != x.alloc() ) {
            using Ip = std::move_iterator<iterator>;
            assign( Ip( x.begin_ ), Ip( x.end_ ) );
        }
        else
            move_assign( x, true_type() );
    }

    constexpr void move_range( pointer from_s, pointer from_e, pointer to ) {
        pointer         old_last = end_;
        difference_type n        = old_last - to;
        {
            pointer              i = from_s + n;
            ConstructTransaction ct( *this, from_e - i );
            for ( ; i < from_e; ++ct.pos, ++i )
                construct( ct.pos, wyne::move( *i ) );
        }
        wyne::move_backward( from_s, from_s + n, old_last );
    }

    template <class Iterator>
    constexpr void insert_with_size( const_iterator position, Iterator first, Iterator last, difference_type n ) {
        pointer p = begin_ + ( position - cbegin() );
        if ( n > 0 ) {
            if ( n <= end_cap() - end_ ) {
                size_type       old_n    = n;
                pointer         old_last = end_;
                Iterator        m        = wyne::next( first, n );
                difference_type diff     = end_ - p;
                if ( n > diff ) {
                    m = first;
                    wyne::advance( m, diff );
                    construct_at_end( m, last, n - diff );
                    n = diff;
                }
                if ( n > 0 ) {
                    move_range( p, old_last, p + old_n );
                    wyne::copy( first, m, p );
                }
            }
            else {
                split_buffer<value_type, allocator_type&> v( recommend( size() + n ), p - begin_, alloc() );
                v.construct_at_end_with_size( first, n );
                p = swap_out_circular_buffer( v, p );
            }
        }
        return make_iter( p );
    }

    template <class Iterator>
    constexpr void insert_with_sentinel( const_iterator position, Iterator first, Iterator last ) {
        difference_type off      = position - cbegin();
        pointer         p        = begin_ + off;
        pointer         old_last = end_;
        for ( ; end_ != end_cap() && first != last; ++first )
            construct_one_at_end( *first );

        split_buffer<value_type, allocator_type&> v( alloc() );
        if ( first != last ) {
            try {
                v.construct_at_end_with_sentinel( wyne::move( first ), wyne::move( last ) );
                size_type old_p    = begin_ - p;
                size_type old_size = old_last - begin_;
                reserve( recommend( size() + v.size() ) );
                p        = begin_ + old_p;
                old_last = begin_ + old_size;
            }
            catch ( ... ) {
                erase( make_iter( old_last ), end() );
                throw;
            }
        }
        p = std::rotate( p, old_last, end_ );
        // TODO: use wyne
        insert( make_iter( p ), std::make_move_iterator( v.begin() ), std::make_move_iterator( v.end() ) );
        return begin() + off;
    }

public:
    constexpr vector() noexcept( std::is_nothrow_default_constructible_v<allocator_type> ) = default;

    constexpr explicit vector( const allocator_type& a ) noexcept : end_cap_( nullptr, a ) {}

    constexpr explicit vector( size_type n ) {
        if WYNE_LIKELY ( n > 0 ) {
            vallocate( n );
            construct_at_end( n );
        }
    }

    constexpr explicit vector( size_type n, const allocator_type& a ) : end_cap_( nullptr, a ) {
        if WYNE_LIKELY ( n > 0 ) {
            vallocate( n );
            construct_at_end( n );
        }
    }

    constexpr vector( size_type n, const value_type& x ) {
        if WYNE_LIKELY ( n > 0 ) {
            vallocate( n );
            construct_at_end( n, x );
        }
    }

    constexpr vector( size_type n, const value_type& x, const allocator_type& a ) : end_cap_( nullptr, a ) {
        if WYNE_LIKELY ( n > 0 ) {
            vallocate( n );
            construct_at_end( n, x );
        }
    }

    template <class InputIterator, std::enable_if_t<is_exactly_input_iterator_t<InputIterator>
                                                        && std::is_constructible_v<value_type, typename iterator_traits<InputIterator>::reference>,
                                                    int> = 0>
    constexpr vector( InputIterator first, InputIterator last ) {
        init_with_sentienl( first, last );
    }

    template <class InputIterator, std::enable_if_t<is_exactly_input_iterator_t<InputIterator>
                                                        && std::is_constructible_v<value_type, typename iterator_traits<InputIterator>::reference>,
                                                    int> = 0>
    constexpr vector( InputIterator first, InputIterator last, const allocator_type& a ) : end_cap_( nullptr, a ) {
        init_with_sentienl( first, last );
    }

    template <class ForwardIterator,
              std::enable_if_t<is_forward_iterator_t<ForwardIterator>
                                   && std::is_constructible_v<value_type, typename iterator_traits<ForwardIterator>::value_type>,
                               int> = 0>
    constexpr vector( ForwardIterator first, ForwardIterator last ) {
        auto n = static_cast<size_type>( wyne::distance( first, last ) );
        init_with_size( first, last, n );
    }

    template <class ForwardIterator,
              std::enable_if_t<is_forward_iterator_t<ForwardIterator>
                                   && std::is_constructible_v<value_type, typename iterator_traits<ForwardIterator>::value_type>,
                               int> = 0>
    constexpr vector( ForwardIterator first, ForwardIterator last, const allocator_type& a ) : end_cap_( nullptr, a ) {
        auto n = static_cast<size_type>( wyne::distance( first, last ) );
        init_with_size( first, last, n );
    }

    constexpr vector( const vector& x ) : end_cap_( nullptr, alloc_traits::select_on_container_copy_construction( x.alloc() ) ) {
        init_with_size( x.begin_, x.end_, x.size() );
    }

    constexpr vector( vector&& x ) noexcept : end_cap_( nullptr, wyne::move( x.alloc() ) ) {
        begin_     = x.begin_;
        end_       = x.end_;
        end_cap_() = x.end_cap();
        x.begin_ = x.end_ = x.end_cap() = nullptr;
    }

    constexpr vector( std::initializer_list<value_type> il ) {
        if ( il.size() > 0 ) {
            vallocate( il.size() );
            construct_at_end( il.begin(), il.end(), il.size() );
        }
    }

    constexpr vector( std::initializer_list<value_type> il, const allocator_type& a ) : end_cap_( nullptr, a ) {
        if ( il.size() > 0 ) {
            vallocate( il.size() );
            construct_at_end( il.begin(), il.end(), il.size() );
        }
    }

    vector& operator=( const vector& x ) {
        if ( this != std::addressof( x ) ) {
            copy_assign_alloc( x );
            assign( x.begin_, x.end_ );
        }
        return *this;
    }

    vector& operator=( vector&& x ) noexcept( alloc_traits::propagate_on_container_move_assignment::value || alloc_traits::is_always_equal::value ) {
        move_assign( x, bool_constant<alloc_traits::propagate_on_container_move_assignment::value>() );
        return *this;
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

    constexpr size_type          capacity() const noexcept { return static_cast<size_type>( end_cap() - begin_ ); }
    constexpr size_type          size() const noexcept { return static_cast<size_type>( end_ - begin_ ); }
    [[nodiscard]] constexpr bool empty() const noexcept { return end_ == begin_; }

    constexpr void clear() noexcept { destruct_at_end( begin_ ); }

    constexpr void resize( size_type n ) {
        const size_type sz = size();
        if ( n > sz )
            append( n - sz );
        else if ( n < sz )
            destruct_at_end( begin_ + n );
    }

    constexpr void resize( size_type n, const_reference x ) {
        const size_type sz = size();
        if ( n > sz )
            append( n - sz, x );
        else if ( n < sz )
            destruct_at_end( begin_ + n );
    }

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

    constexpr iterator erase( const_iterator position ) {
        pointer p = begin_ + ( position - cbegin() );
        destruct_at_end( wyne::move( p + 1, end_, p ) );
        return make_iter( p );
    }

    constexpr iterator erase( const_iterator first, const_iterator last ) {
        pointer p = begin_ + ( first - cbegin() );
        if ( first != last )
            destruct_at_end( wyne::move( p + ( last - first ), end_, p ) );
        return make_iter( p );
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

    template <class InputIterator, std::enable_if_t<is_exactly_input_iterator_t<InputIterator>
                                                        && std::is_constructible_v<value_type, typename iterator_traits<InputIterator>::value_type>,
                                                    int> = 0>
    constexpr void assign( InputIterator first, InputIterator last ) {
        clear();
        for ( ; first != last; ++first )
            emplace_back( *first );
    }

    template <class ForwardIterator,
              std::enable_if_t<is_forward_iterator_t<ForwardIterator>
                                   && std::is_constructible_v<value_type, typename iterator_traits<ForwardIterator>::value_type>,
                               int> = 0>
    constexpr void assign( ForwardIterator first, ForwardIterator last ) {
        const auto new_size = static_cast<size_type>( wyne::distance( first, last ) );
        if ( new_size <= capacity() ) {
            if ( new_size <= size() ) {
                pointer m = wyne::copy( first, last, begin_ );
                destruct_at_end( m );
            }
            else {
                const auto mid = wyne::next( first, size() );
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

    constexpr iterator insert( const_iterator position, const_reference x ) {
        pointer p = begin_ + ( position - cbegin() );
        if ( end_ < end_cap() ) {
            if ( p == end_ )
                construct_one_at_end( x );
            else {
                move_range( p, end_, p + 1 );
                // TODO: use wyne in the future
                const_pointer xr = std::pointer_traits<const_pointer>::pointer_to( x );
                if ( std::__is_pointer_in_range( std::to_address( p ), std::to_address( end_ ), std::to_address( xr ) ) )
                    ++xr;
                *p = *xr;
            }
        }
        else {
            split_buffer<value_type, allocator_type&> v( recommend( size() + 1 ), p - begin_, alloc() );
            v.push_back( x );
            p = swap_out_circular_buffer( v, p );
        }
        return make_iter( p );
    }

    constexpr iterator insert( const_iterator position, value_type&& x ) {
        pointer p = begin_ + ( position - cbegin() );
        if ( end_ < end_cap() ) {
            if ( p == end_ )
                construct_one_at_end( wyne::move( x ) );
            else {
                move_range( p, end_, p + 1 );
                *p = wyne::move( x );
            }
        }
        else {
            split_buffer<value_type, allocator_type&> v( recommend( size() + 1 ), p - begin_, alloc() );
            v.push_back( wyne::move( x ) );
            p = swap_out_circular_buffer( v, p );
        }
        return make_iter( p );
    }

    template <class... Args>
    constexpr iterator emplace( const_iterator position, Args&&... args ) {
        pointer p = begin_ + ( position - cbegin() );
        if ( end_ < end_cap() ) {
            if ( p == end_ )
                construct_one_at_end( wyne::forward<Args>( args )... );
            else {
                move_range( p, end_, p + 1 );
                // HACK: construct can throw
                destroy( p );
                construct( p, wyne::forward<Args>( args )... );
            }
        }
        else {
            split_buffer<value_type, allocator_type&> v( recommend( size() + 1 ), p - begin_, alloc() );
            v.unsafe_emplace_back( wyne::forward<Args>( args )... );
            p = swap_out_circular_buffer( v, p );
        }
        return make_iter( p );
    }

    constexpr iterator insert( const_iterator position, size_type n, const_reference x ) {
        pointer p = begin_ + ( position - cbegin() );
        if ( n > 0 ) {
            if ( n <= static_cast<size_type>( end_cap() - end_ ) ) {
                size_type old_n    = n;
                pointer   old_last = end_;
                if ( n > static_cast<size_type>( end_ - p ) ) {
                    size_type as = n - ( end_ - p );
                    construct_at_end( as, x );
                    n -= as;
                }
                if ( n > 0 ) {
                    move_range( p, old_last, p + old_n );
                    // TODO: use wyne in the future
                    const_pointer xr = std::pointer_traits<pointer>::pointer_to( x );
                    if ( p <= xr && xr < end_ )
                        xr += old_n;
                    wyne::fill_n( p, n, *xr );
                }
            }
            else {
                split_buffer<value_type, allocator_type&> v( recommend( size() + n ), p - begin_, alloc() );
                v.construct_at_end( n, x );
                p = swap_out_circular_buffer( v, p );
            }
        }
        return make_iter( p );
    }

    template <class ForwardIterator, std::enable_if_t<is_forward_iterator_t<ForwardIterator>, int> = 0>
    constexpr iterator insert( const_iterator position, ForwardIterator first, ForwardIterator last ) {
        return insert_with_size( position, first, last, wyne::distance( first, last ) );
    }

    constexpr void swap( vector& x ) noexcept {
        wyne::swap( begin_, x.begin_ );
        wyne::swap( end_, x.end_ );
        wyne::swap( end_cap(), x.end_cap() );
        wyne::swap_allocator( alloc(), x.alloc() );
    }

    constexpr void print( std::ostream& os, const std::string& suffix = "", const std::string& name = "Vector" ) const {
        os << name << " {size=" << size() << ", capacity=" << capacity() << ", elements=[";
        auto it = begin_;
        if ( it != nullptr )
            os << *it++;
        for ( ; it != end_; ++it ) {
            os << ", " << *it;
        }
        os << "]}" << suffix;
    }
};

template <class Iterator, class Allocator = wyne::allocator<iter_value_type<Iterator>>>
vector( Iterator, Iterator, Allocator = Allocator() ) -> vector<iter_value_type<Iterator>, Allocator>;

template <class Tp, class Allocator>
inline constexpr void swap( vector<Tp, Allocator>& x, vector<Tp, Allocator>& y ) noexcept {
    x.swap( y );
}

template <class Tp, class Allocator>
std::ostream& operator<<( std::ostream& os, const vector<Tp, Allocator>& v ) {
    v.print( os );
    return os;
}

// operator

template <class Tp, class Allocator>
inline constexpr bool operator==( const vector<Tp, Allocator>& x, const vector<Tp, Allocator>& y ) {
    return x.size() == y.size() && wyne::equal( x.begin(), x.end(), y.begin() );
}

template <class Tp, class Allocator>
inline constexpr bool operator!=( const vector<Tp, Allocator>& x, const vector<Tp, Allocator>& y ) {
    return !( x == y );
}

template <class Tp, class Allocator>
inline constexpr bool operator<( const vector<Tp, Allocator>& x, const vector<Tp, Allocator>& y ) {
    return wyne::lexicographical_compare( x.begin(), x.end(), y.begin(), y.end() );
}

template <class Tp, class Allocator>
inline constexpr bool operator>( const vector<Tp, Allocator>& x, const vector<Tp, Allocator>& y ) {
    return y < x;
}

template <class Tp, class Allocator>
inline constexpr bool operator<=( const vector<Tp, Allocator>& x, const vector<Tp, Allocator>& y ) {
    return !( x > y );
}

template <class Tp, class Allocator>
inline constexpr bool operator>=( const vector<Tp, Allocator>& x, const vector<Tp, Allocator>& y ) {
    return !( x < y );
}

// TODO: use wyne

template <class Tp, class Allocator, class Up>
inline constexpr typename vector<Tp, Allocator>::size_type erase( vector<Tp, Allocator>& v, const Up& u ) {
    auto old_size = v.size();
    v.erase( std::remove( v.begin(), v.end(), u ), v.end() );
    return old_size - v.size();
}

template <class Tp, class Allocator, class Pred>
inline constexpr typename vector<Tp, Allocator>::size_type erase_if( vector<Tp, Allocator>& v, Pred pred ) {
    auto old_size = v.size();
    v.erase( std::remove_if( v.begin(), v.end(), pred ), v.end() );
    return old_size - v.size();
}

};  // namespace wyne

#endif