#include "vector.h"
#include <gtest/gtest.h>

// 命名空间别名简化写法
namespace my = wyne;

// 测试默认构造
TEST( VectorTest, DefaultConstructor ) {
    my::vector<int> v;
    EXPECT_EQ( v.size(), 0 );
    EXPECT_EQ( v.capacity(), 0 );
    EXPECT_TRUE( v.empty() );
}

// 测试带大小的构造
TEST( VectorTest, ConstructorWithSize ) {
    my::vector<int> v( 5 );
    EXPECT_EQ( v.size(), 5 );
    for ( const auto& val : v ) {
        EXPECT_EQ( val, int{} );
    }
}

// 测试带大小和初始值的构造
TEST( VectorTest, ConstructorWithSizeAndValue ) {
    my::vector<int> v( 5, 42 );
    EXPECT_EQ( v.size(), 5 );
    for ( const auto& val : v ) {
        EXPECT_EQ( val, 42 );
    }
}

// 测试拷贝构造
TEST( VectorTest, CopyConstructor ) {
    my::vector<int> v1( 3, 10 );
    my::vector<int> v2( v1 );

    EXPECT_EQ( v1.size(), v2.size() );
    for ( size_t i = 0; i < v1.size(); ++i ) {
        EXPECT_EQ( v1[ i ], v2[ i ] );
    }

    // 修改原对象不应影响拷贝对象
    v1[ 0 ] = 99;
    EXPECT_NE( v1[ 0 ], v2[ 0 ] );
}

// 测试 push_back 扩容
TEST( VectorTest, PushBackResizes ) {
    my::vector<int> v;
    for ( int i = 0; i < 100; ++i ) {
        v.push_back( i );
    }

    EXPECT_EQ( v.size(), 100u );
    EXPECT_GE( v.capacity(), 100u );

    for ( int i = 0; i < 100; ++i ) {
        EXPECT_EQ( v[ i ], i );
    }
}

// 测试下标访问
TEST( VectorTest, AccessOperators ) {
    my::vector<int> v( 3, 10 );
    EXPECT_EQ( v[ 0 ], 10 );
    EXPECT_EQ( v[ 1 ], 10 );
    EXPECT_EQ( v[ 2 ], 10 );

    v[ 1 ] = 20;
    EXPECT_EQ( v[ 1 ], 20 );
}

// 测试 at() 异常
TEST( VectorTest, AtThrowsOnOutOfBounds ) {
    my::vector<int> v( 3, 10 );
    EXPECT_NO_THROW( v.at( 2 ) );
    EXPECT_ANY_THROW( v.at( 100 ) );
}

// 测试 clear
TEST( VectorTest, Clear ) {
    std::vector<int> v( 5, 42 );
    v.clear();
    EXPECT_EQ( v.size(), 0 );
    EXPECT_FALSE( v.empty() );  // capacity 可能不为0
}
