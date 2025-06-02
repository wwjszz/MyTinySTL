#ifndef WYNE_SPLIT_BUFFER_H__
#define WYNE_SPLIT_BUFFER_H__

#include <memory>
#include <type_traits>

#include "allocator.h"
#include "compressed_pair.h"

namespace wyne {

// Currently only used for vector. It may be completed in the future.

template <class Tp, class Allocator = allocator<Tp>>
class split_buffer {
    static_assert( std::is_lvalue_reference<Allocator>::value,
                   "Allocator must be an lvalue reference as only being used by vector." );

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

    pointer                                  first_;
    pointer                                  begin_;
    pointer                                  end_;
    compressed_pair<pointer, allocator_type> end_cap_;
};

}  // namespace wyne

#endif