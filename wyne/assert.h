#ifndef WYNE_ASSERT_H__
#define WYNE_ASSERT_H__

#define WYNE_ASSERT( expression, message )                            \
    do {                                                              \
        if ( __builtin_expect( static_cast<bool>( expression ), 1 ) ) \
    } while ( 0 )

#ifdef WYNE_CHECK_ENABLED

#define WYNE_ASSERT_VALID_ELEMENT_ACCESS( expr, message )

#else

#define WYNE_ASSERT_VALID_ELEMENT_ACCESS( expr, message )

#endif

#endif