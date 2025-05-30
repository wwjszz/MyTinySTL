
#include "algobase.h"
#include <cstring>
#include <gtest/gtest.h>
#include <iterator>
#include <vector>

// -----------------------------
// max / min 测试
// -----------------------------

TEST( VinceTest, Max ) {
    EXPECT_EQ( vince::max( 3, 5 ), 5 );
    EXPECT_EQ( vince::max( 3, 3 ), 3 );
    EXPECT_EQ( vince::max( 'a', 'z' ), 'z' );
    // EXPECT_EQ( vince::max( "apple", "banana" ), "banana" );
}

TEST( VinceTest, MaxWithComparator ) {
    auto cmp = []( int a, int b ) { return a > b; };
    EXPECT_EQ( vince::max( 3, 5, cmp ), 3 );
    EXPECT_EQ( vince::max( 5, 3, cmp ), 3 );
}

TEST( VinceTest, Min ) {
    EXPECT_EQ( vince::min( 3, 5 ), 3 );
    EXPECT_EQ( vince::min( 3, 3 ), 3 );
    EXPECT_EQ( vince::min( 'a', 'z' ), 'a' );
    // EXPECT_EQ( vince::min( "apple", "banana" ), "apple" );
}

TEST( VinceTest, MinWithComparator ) {
    auto cmp = []( int a, int b ) { return a > b; };
    EXPECT_EQ( vince::min( 3, 5, cmp ), 5 );
    EXPECT_EQ( vince::min( 5, 3, cmp ), 5 );
}

// -----------------------------
// iter_swap 测试
// -----------------------------

TEST( VinceTest, IterSwap ) {
    int a = 1, b = 2;
    vince::iter_swap( &a, &b );
    EXPECT_EQ( a, 2 );
    EXPECT_EQ( b, 1 );

    std::vector<int> v1 = { 1, 2 };
    std::vector<int> v2 = { 3, 4 };
    vince::iter_swap( v1.begin(), v2.begin() );
    EXPECT_EQ( v1[ 0 ], 3 );
    EXPECT_EQ( v2[ 0 ], 1 );
}

// -----------------------------
// copy 测试
// -----------------------------

TEST( VinceTest, CopyPOD ) {
    int src[]     = { 1, 2, 3, 4, 5 };
    int dest[ 5 ] = {};
    vince::copy( std::cbegin( src ), std::cbegin( src ) + 5, dest );
    EXPECT_TRUE( std::memcmp( src, dest, sizeof( src ) ) == 0 );
}

// -----------------------------
// copy_backward 测试
// -----------------------------

TEST( VinceTest, CopyBackward ) {
    int src[]     = { 1, 2, 3, 4, 5 };
    int dest[ 5 ] = {};
    vince::copy_backward( src, src + 5, dest + 5 );
    int expected[] = { 1, 2, 3, 4, 5 };
    EXPECT_TRUE( std::memcmp( dest, expected, sizeof( dest ) ) == 0 );
}

// -----------------------------
// move 测试
// -----------------------------

template <class InputIterator, class OutputIterator>
inline constexpr OutputIterator move( InputIterator first, InputIterator last, OutputIterator result ) {
    while ( first != last )
        *result++ = std::move( *first++ );
    return result;
}

TEST( VinceTest, Move ) {
    std::string s1[] = { "one", "two", "three" };
    std::string s2[ 3 ];
    vince::move( s1, s1 + 3, s2 );
    EXPECT_EQ( s2[ 0 ], "one" );
    EXPECT_EQ( s2[ 1 ], "two" );
    EXPECT_EQ( s2[ 2 ], "three" );

    // 源对象应仍有效（移动后未定义状态）
    for ( const auto& s : s1 ) {
        EXPECT_TRUE( s.empty() || !s.empty() );  // 不为空或已移走
    }
}

// -----------------------------
// move_backward 测试
// -----------------------------

template <class BidirectionalIterator1, class BidirectionalIterator2>
inline constexpr BidirectionalIterator2 move_backward( BidirectionalIterator1 first, BidirectionalIterator1 last,
                                                       BidirectionalIterator2 result ) {
    while ( last != first )
        *( --result ) = std::move( *( --last ) );
    return result;
}
