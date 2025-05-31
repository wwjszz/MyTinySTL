#ifndef VINCE_CONFIG_H__
#define VINCE_CONFIG_H__

#define VINCE_LIKELY( x ) ( __builtin_expect( !!( x ), 1 ) )
#define VINCE_UNLIKELY( x ) ( __builtin_expect( !!( x ), 0 ) )

namespace vince {

// new file?

template <class T>
inline constexpr bool requires_special_alignment() {
    return alignof( T ) > __STDCPP_DEFAULT_NEW_ALIGNMENT__;
}

};  // namespace vince
#endif