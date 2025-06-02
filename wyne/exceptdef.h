#ifndef WYNE_EXCEPTDEF_H__
#define WYNE_EXCEPTDEF_H__

#include <cassert>
#include <stdexcept>

namespace wyne {

#define WYNE_DEBUG( expr ) assert( expr )

#define WYNE_THROW_LENGTH_ERROR_IF( expr, what ) \
    do {                                         \
        if ( ( expr ) )                          \
            throw std::length_error( what );     \
    } while ( 0 )

#define WYNE_THROW_OUT_OF_RANGE_IF( expr, what ) \
    do {                                         \
        if ( ( expr ) )                          \
            throw std::out_of_range( what );     \
    } while ( 0 )

#define WYNE_THROW_RUNTIME_ERROR_IF( expr, what ) \
    do {                                          \
        if ( ( expr ) )                           \
            throw std::runtime_error( what );     \
    } while ( 0 )

};  // namespace wyne

#endif