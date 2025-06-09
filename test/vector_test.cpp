#include "iterator.h"
#include "vector.h"  // 替换为你实际的头文件路径
#include <algorithm>
#include <gtest/gtest.h>
#include <stdexcept>

// 使用命名空间
using namespace wyne;

// 测试默认构造函数和基础操作
TEST( VectorTest, DefaultConstructorAndBasicOperations ) {
    vector<int> vec;
    EXPECT_TRUE( vec.empty() );
    EXPECT_EQ( vec.size(), 0 );
    EXPECT_EQ( vec.capacity(), 0 );

    vec.push_back( 1 );
    EXPECT_EQ( vec.size(), 1 );
    EXPECT_EQ( vec[ 0 ], 1 );
    EXPECT_FALSE( vec.empty() );
}

// 测试带初始大小的构造函数
TEST( VectorTest, ConstructorWithSize ) {
    vector<int> vec( 5, 3 );
    EXPECT_EQ( vec.size(), 5 );
    EXPECT_EQ( vec.capacity(), 5 );
    for ( size_t i = 0; i < 5; ++i ) {
        EXPECT_EQ( vec[ i ], 3 );
    }
}

// 测试元素访问方法（operator[], at(), front(), back()）
TEST( VectorTest, ElementAccess ) {
    vector<int> vec = { 1, 2, 3, 4, 5 };
    EXPECT_EQ( vec[ 0 ], 1 );
    EXPECT_EQ( vec.at( 2 ), 3 );
    EXPECT_EQ( vec.front(), 1 );
    EXPECT_EQ( vec.back(), 5 );

    vec.print( std::cout, "\n" );

    // 测试 at() 越界异常
    EXPECT_THROW( vec.at( 10 ), std::out_of_range );
}

// 测试 push_back 和扩容逻辑
TEST( VectorTest, PushBackAndResizing ) {
    vector<int> vec;
    size_t      initial_capacity = vec.capacity();

    vec.push_back( 1 );
    EXPECT_GT( vec.capacity(), initial_capacity );  // 容量应增加

    vec.push_back( 2 );
    vec.push_back( 3 );
    EXPECT_EQ( vec.size(), 3 );
    EXPECT_EQ( vec[ 2 ], 3 );
}

// 测试 pop_back
TEST( VectorTest, PopBack ) {
    vector<int> vec = { 1, 2, 3 };
    vec.pop_back();
    EXPECT_EQ( vec.size(), 2 );
    EXPECT_EQ( vec[ 1 ], 2 );

    EXPECT_NO_THROW( vec.pop_back() );
    EXPECT_NO_THROW( vec.pop_back() );
    EXPECT_TRUE( vec.empty() );
    // EXPECT_THROW( vec.pop_back(), std::out_of_range );  // 空 vector 抛异常
}

// 测试 resize
TEST( VectorTest, Resize ) {
    vector<int> vec( 3, 1 );
    vec.resize( 5, 2 );
    EXPECT_EQ( vec.size(), 5 );
    EXPECT_EQ( vec[ 3 ], 2 );
    EXPECT_EQ( vec[ 4 ], 2 );

    vec.resize( 2 );
    EXPECT_EQ( vec.size(), 2 );
    // EXPECT_EQ( vec.capacity(), 2 );  // 假设 resize 不减少容量
}

// 测试 clear
TEST( VectorTest, Clear ) {
    vector<int> vec = { 1, 2, 3 };
    vec.clear();
    EXPECT_TRUE( vec.empty() );
    EXPECT_EQ( vec.size(), 0 );
    EXPECT_NE( vec.capacity(), 0 );  // 假设 clear 不释放内存
}

// 测试拷贝构造和赋值
TEST( VectorTest, CopyConstructorAndAssignment ) {
    vector<int> original = { 1, 2, 3 };
    vector<int> copy     = original;
    EXPECT_EQ( copy.size(), 3 );
    for ( size_t i = 0; i < 3; ++i ) {
        EXPECT_EQ( copy[ i ], original[ i ] );
    }

    vector<int> assign;
    assign = original;
    EXPECT_EQ( assign.size(), 3 );
    for ( size_t i = 0; i < 3; ++i ) {
        EXPECT_EQ( assign[ i ], original[ i ] );
    }
}

// 测试迭代器
TEST( VectorTest, Iterators ) {
    vector<int> vec = { 1, 2, 3 };
    auto        it  = vec.begin();
    for ( int val : vec ) {
        EXPECT_EQ( *it, val );
        ++it;
    }
    EXPECT_EQ( it, vec.end() );
}

// 测试异常安全
TEST( VectorTest, ExceptionSafety ) {
    vector<int> vec( 3, 0 );
    EXPECT_THROW( vec.at( 5 ), std::out_of_range );
    EXPECT_NO_THROW( vec.at( 2 ) );
}

// 主函数运行所有测试
int main( int argc, char** argv ) {
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}