#include "vector.h"  // 包含你的vector实现
#include <algorithm>
#include <gtest/gtest.h>
#include <ostream>
#include <stdexcept>
#include <vector>

const size_t HUGE_SIZE   = 1000000;  // 百万级数据量
const size_t MEDIUM_SIZE = 10000;    // 中等数据量

class VectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 只填充非const向量
        for ( int i = 0; i < 1000; ++i ) {
            vec.push_back( i );
        }
    }

    // 各种测试向量
    wyne::vector<int> empty_vec;
    wyne::vector<int> vec;
    wyne::vector<int> move_source{ 1, 2, 3 };
};

// 创建const向量的辅助函数
const wyne::vector<int> create_const_vector() {
    wyne::vector<int> temp;
    for ( int i = 0; i < 1000; ++i ) {
        temp.push_back( i );
    }
    return temp;
}

// 测试所有构造函数
TEST_F( VectorTest, Constructors ) {
    // 默认构造函数
    EXPECT_TRUE( empty_vec.empty() );
    EXPECT_EQ( empty_vec.size(), 0 );
    EXPECT_GE( empty_vec.capacity(), 0 );

    // 大小构造函数
    wyne::vector<int> size_vec( 100 );
    EXPECT_EQ( size_vec.size(), 100 );
    EXPECT_GE( size_vec.capacity(), 100 );
    for ( size_t i = 0; i < 100; ++i ) {
        EXPECT_EQ( size_vec[ i ], int() );
    }

    // 大小+值构造函数
    wyne::vector<int> size_value_vec( 50, 42 );
    EXPECT_EQ( size_value_vec.size(), 50 );
    for ( const auto& val : size_value_vec ) {
        EXPECT_EQ( val, 42 );
    }

    // 范围构造函数
    int               arr[] = { 1, 2, 3, 4, 5 };
    wyne::vector<int> range_vec( arr, arr + 5 );
    EXPECT_EQ( range_vec.size(), 5 );
    for ( size_t i = 0; i < 5; ++i ) {
        EXPECT_EQ( range_vec[ i ], arr[ i ] );
    }

    // 拷贝构造函数
    wyne::vector<int> copy_vec( vec );
    EXPECT_EQ( copy_vec.size(), vec.size() );
    for ( size_t i = 0; i < vec.size(); ++i ) {
        EXPECT_EQ( copy_vec[ i ], vec[ i ] );
    }

    // 移动构造函数
    wyne::vector<int> temp{ 1, 2, 3 };
    wyne::vector<int> move_vec( wyne::move( temp ) );
    EXPECT_EQ( move_vec.size(), 3 );
    EXPECT_TRUE( temp.empty() );
    EXPECT_EQ( move_vec[ 0 ], 1 );
    EXPECT_EQ( move_vec[ 1 ], 2 );
    EXPECT_EQ( move_vec[ 2 ], 3 );

    // 初始化列表构造函数
    wyne::vector<int> il_vec{ 10, 20, 30, 40 };
    EXPECT_EQ( il_vec.size(), 4 );
    EXPECT_EQ( il_vec[ 0 ], 10 );
    EXPECT_EQ( il_vec[ 1 ], 20 );
    EXPECT_EQ( il_vec[ 2 ], 30 );
    EXPECT_EQ( il_vec[ 3 ], 40 );
}

// 测试赋值运算符
TEST_F( VectorTest, AssignmentOperators ) {
    // 拷贝赋值
    wyne::vector<int> copy_assigned;
    copy_assigned = vec;
    EXPECT_EQ( copy_assigned.size(), vec.size() );
    for ( size_t i = 0; i < vec.size(); ++i ) {
        EXPECT_EQ( copy_assigned[ i ], vec[ i ] );
    }

    // 移动赋值
    wyne::vector<int> move_assigned;
    move_assigned = wyne::move( move_source );
    EXPECT_EQ( move_assigned.size(), 3 );
    EXPECT_TRUE( move_source.empty() );
    EXPECT_EQ( move_assigned[ 0 ], 1 );
    EXPECT_EQ( move_assigned[ 1 ], 2 );
    EXPECT_EQ( move_assigned[ 2 ], 3 );

    // 初始化列表赋值
    wyne::vector<int> il_assigned;
    il_assigned = { 100, 200, 300 };
    EXPECT_EQ( il_assigned.size(), 3 );
    EXPECT_EQ( il_assigned[ 0 ], 100 );
    EXPECT_EQ( il_assigned[ 1 ], 200 );
    EXPECT_EQ( il_assigned[ 2 ], 300 );
}

// 测试元素访问函数
TEST_F( VectorTest, ElementAccess ) {
    // operator[]
    for ( size_t i = 0; i < vec.size(); ++i ) {
        EXPECT_EQ( vec[ i ], static_cast<int>( i ) );
    }

    // const operator[]
    const wyne::vector<int> const_vec = create_const_vector();
    for ( size_t i = 0; i < const_vec.size(); ++i ) {
        EXPECT_EQ( const_vec[ i ], static_cast<int>( i ) );
    }

    // at()
    for ( size_t i = 0; i < vec.size(); ++i ) {
        EXPECT_EQ( vec.at( i ), static_cast<int>( i ) );
    }

    // const at()
    for ( size_t i = 0; i < const_vec.size(); ++i ) {
        EXPECT_EQ( const_vec.at( i ), static_cast<int>( i ) );
    }

    // 越界访问测试
    // EXPECT_THROW( vec.at( vec.size() ), std::out_of_range );
    // EXPECT_THROW( empty_vec.at( 0 ), std::out_of_range );

    // front()
    EXPECT_EQ( vec.front(), 0 );
    EXPECT_EQ( const_vec.front(), 0 );

    // back()
    EXPECT_EQ( vec.back(), 999 );
    EXPECT_EQ( const_vec.back(), 999 );

    // data()
    EXPECT_NE( vec.data(), nullptr );
    EXPECT_EQ( *( vec.data() ), 0 );
    EXPECT_EQ( *( vec.data() + 1 ), 1 );

    // const data()
    EXPECT_NE( const_vec.data(), nullptr );
    EXPECT_EQ( *( const_vec.data() ), 0 );
}

// 测试迭代器
TEST_F( VectorTest, Iterators ) {
    // 非const迭代器
    int count = 0;
    for ( auto it = vec.begin(); it != vec.end(); ++it ) {
        EXPECT_EQ( *it, count++ );
    }
    EXPECT_EQ( count, 1000 );

    // const迭代器
    const wyne::vector<int> const_vec = create_const_vector();
    count                             = 0;
    // wyne::cout << "const_ver" << const_vec[ 0 ] << wyne::endl;
    for ( auto it = const_vec.begin(); it != const_vec.end(); ++it ) {
        // wyne::cout << "it:" << *it << wyne::endl;
        EXPECT_EQ( *it, count++ );
    }
    EXPECT_EQ( count, 1000 );

    // 反向迭代器
    count = 999;
    for ( auto rit = vec.rbegin(); rit != vec.rend(); ++rit ) {
        EXPECT_EQ( *rit, count-- );
    }
    EXPECT_EQ( count, -1 );

    // const反向迭代器
    count = 999;
    for ( auto rit = const_vec.rbegin(); rit != const_vec.rend(); ++rit ) {
        EXPECT_EQ( *rit, count-- );
    }
    EXPECT_EQ( count, -1 );

    // 空向量迭代器
    EXPECT_EQ( empty_vec.begin(), empty_vec.end() );
    EXPECT_EQ( empty_vec.rbegin(), empty_vec.rend() );
}

// 测试容量函数
TEST_F( VectorTest, CapacityFunctions ) {
    // empty()
    EXPECT_TRUE( empty_vec.empty() );
    EXPECT_FALSE( vec.empty() );

    // size()
    EXPECT_EQ( empty_vec.size(), 0 );
    EXPECT_EQ( vec.size(), 1000 );

    // max_size()
    EXPECT_GT( vec.max_size(), 0 );

    // capacity()
    EXPECT_GE( vec.capacity(), vec.size() );
    EXPECT_GE( empty_vec.capacity(), 0 );

    // reserve()
    size_t old_capacity = vec.capacity();
    vec.reserve( old_capacity + 100 );
    EXPECT_GE( vec.capacity(), old_capacity + 100 );
    for ( size_t i = 0; i < vec.size(); ++i ) {
        EXPECT_EQ( vec[ i ], static_cast<int>( i ) );
    }

    // shrink_to_fit()
    wyne::vector<int> shrink_vec{ 1, 2, 3 };
    shrink_vec.reserve( 100 );
    EXPECT_GE( shrink_vec.capacity(), 100 );
    shrink_vec.shrink_to_fit();
    EXPECT_EQ( shrink_vec.capacity(), shrink_vec.size() );
}

// 测试修改器函数
TEST_F( VectorTest, Modifiers ) {
    // clear()
    wyne::vector<int> clear_vec{ 1, 2, 3 };
    clear_vec.clear();
    EXPECT_TRUE( clear_vec.empty() );
    EXPECT_EQ( clear_vec.size(), 0 );
    EXPECT_GE( clear_vec.capacity(), 3 );  // 容量可能不变

    // insert() - 单元素
    auto it = vec.insert( vec.begin() + 500, 42 );
    EXPECT_EQ( vec.size(), 1001 );
    EXPECT_EQ( *it, 42 );
    EXPECT_EQ( vec[ 500 ], 42 );
    EXPECT_EQ( vec[ 499 ], 499 );
    EXPECT_EQ( vec[ 501 ], 500 );

    // insert() - 多个相同元素
    it = vec.insert( vec.begin() + 100, 3, 99 );
    EXPECT_EQ( vec.size(), 1004 );
    EXPECT_EQ( *it, 99 );
    EXPECT_EQ( vec[ 100 ], 99 );
    EXPECT_EQ( vec[ 101 ], 99 );
    EXPECT_EQ( vec[ 102 ], 99 );
    EXPECT_EQ( vec[ 103 ], 100 );  // 原位置元素后移

    // insert() - 范围
    int insert_arr[] = { 201, 202, 203 };
    it               = vec.insert( vec.begin() + 200, insert_arr, insert_arr + 3 );
    EXPECT_EQ( vec.size(), 1007 );
    EXPECT_EQ( *it, 201 );
    EXPECT_EQ( vec[ 200 ], 201 );
    EXPECT_EQ( vec[ 201 ], 202 );
    EXPECT_EQ( vec[ 202 ], 203 );
    EXPECT_EQ( vec[ 203 ], 197 );  // 原位置元素后移

    // emplace()
    struct TestData {
        int    a;
        double b;
        TestData( int x, double y ) : a( x ), b( y ) {}
    };

    wyne::vector<TestData> complex_vec;
    complex_vec.emplace( complex_vec.end(), 1, 1.1 );
    complex_vec.emplace( complex_vec.end(), 2, 2.2 );

    auto cit = complex_vec.emplace( complex_vec.begin(), 3, 3.3 );
    EXPECT_EQ( complex_vec.size(), 3 );
    EXPECT_EQ( cit->a, 3 );
    EXPECT_DOUBLE_EQ( cit->b, 3.3 );
    EXPECT_EQ( complex_vec[ 0 ].a, 3 );
    EXPECT_EQ( complex_vec[ 1 ].a, 1 );
    EXPECT_EQ( complex_vec[ 2 ].a, 2 );

    // vec = create_const_vector();

    // erase() - 单元素
    it = vec.erase( vec.begin() + 600 );
    EXPECT_EQ( vec.size(), 1006 );
    EXPECT_EQ( *it, 594 );         // 删除位置后的元素前移
    EXPECT_EQ( vec[ 600 ], 594 );  // 原601元素前移到600位置

    // erase() - 范围
    it = vec.erase( vec.begin() + 700, vec.begin() + 705 );
    EXPECT_EQ( vec.size(), 1001 );
    EXPECT_EQ( *it, 699 );         // 删除后第一个元素
    EXPECT_EQ( vec[ 700 ], 699 );  // 原705元素前移到700位置

    // push_back()
    wyne::vector<int> push_vec;
    for ( int i = 0; i < 1000; ++i ) {
        push_vec.push_back( i );
        EXPECT_EQ( push_vec[ i ], i );
    }
    EXPECT_EQ( push_vec.size(), 1000 );

    // emplace_back()
    wyne::vector<TestData> emp_vec;
    emp_vec.emplace_back( 10, 10.5 );
    emp_vec.emplace_back( 20, 20.5 );
    EXPECT_EQ( emp_vec.size(), 2 );
    EXPECT_EQ( emp_vec[ 0 ].a, 10 );
    EXPECT_DOUBLE_EQ( emp_vec[ 0 ].b, 10.5 );
    EXPECT_EQ( emp_vec[ 1 ].a, 20 );
    EXPECT_DOUBLE_EQ( emp_vec[ 1 ].b, 20.5 );

    // pop_back()
    wyne::vector<int> pop_vec{ 1, 2, 3 };
    pop_vec.pop_back();
    EXPECT_EQ( pop_vec.size(), 2 );
    EXPECT_EQ( pop_vec.back(), 2 );
    pop_vec.pop_back();
    EXPECT_EQ( pop_vec.size(), 1 );
    EXPECT_EQ( pop_vec.back(), 1 );
    pop_vec.pop_back();
    EXPECT_TRUE( pop_vec.empty() );
    // EXPECT_THROW( pop_vec.pop_back(), wyne::out_of_range );  // 测试空向量pop

    // resize() - 增大
    wyne::vector<int> resize_vec{ 1, 2, 3 };
    resize_vec.resize( 5, 42 );
    EXPECT_EQ( resize_vec.size(), 5 );
    EXPECT_EQ( resize_vec[ 0 ], 1 );
    EXPECT_EQ( resize_vec[ 1 ], 2 );
    EXPECT_EQ( resize_vec[ 2 ], 3 );
    EXPECT_EQ( resize_vec[ 3 ], 42 );
    EXPECT_EQ( resize_vec[ 4 ], 42 );

    // resize() - 缩小
    resize_vec.resize( 2 );
    EXPECT_EQ( resize_vec.size(), 2 );
    EXPECT_EQ( resize_vec[ 0 ], 1 );
    EXPECT_EQ( resize_vec[ 1 ], 2 );

    // resize() - 空值增大
    empty_vec.resize( 10 );
    EXPECT_EQ( empty_vec.size(), 10 );
    for ( const auto& val : empty_vec ) {
        EXPECT_EQ( val, int() );
    }

    // swap()
    wyne::vector<int> swap1{ 1, 2, 3 };
    wyne::vector<int> swap2{ 4, 5, 6, 7 };
    swap1.swap( swap2 );
    EXPECT_EQ( swap1.size(), 4 );
    EXPECT_EQ( swap2.size(), 3 );
    EXPECT_EQ( swap1[ 0 ], 4 );
    EXPECT_EQ( swap1[ 3 ], 7 );
    EXPECT_EQ( swap2[ 0 ], 1 );
    EXPECT_EQ( swap2[ 2 ], 3 );
}

// 测试比较运算符
TEST_F( VectorTest, ComparisonOperators ) {
    wyne::vector<int> v1{ 1, 2, 3 };
    wyne::vector<int> v2{ 1, 2, 3 };
    wyne::vector<int> v3{ 1, 2, 4 };
    wyne::vector<int> v4{ 1, 2 };
    wyne::vector<int> v5{ 1, 2, 3, 4 };

    // operator==
    EXPECT_TRUE( v1 == v2 );
    EXPECT_FALSE( v1 == v3 );

    // operator!=
    EXPECT_TRUE( v1 != v3 );
    EXPECT_FALSE( v1 != v2 );

    // operator<
    EXPECT_TRUE( v4 < v1 );
    EXPECT_TRUE( v1 < v3 );
    EXPECT_TRUE( v1 < v5 );
    EXPECT_FALSE( v1 < v2 );
    EXPECT_FALSE( v3 < v1 );

    // operator<=
    EXPECT_TRUE( v1 <= v2 );
    EXPECT_TRUE( v4 <= v1 );
    EXPECT_FALSE( v3 <= v1 );

    // operator>
    EXPECT_TRUE( v3 > v1 );
    EXPECT_TRUE( v1 > v4 );
    EXPECT_FALSE( v1 > v2 );
    EXPECT_FALSE( v1 > v3 );

    // operator>=
    EXPECT_TRUE( v1 >= v2 );
    EXPECT_TRUE( v3 >= v1 );
    EXPECT_FALSE( v1 >= v5 );
}

// 大数据量测试 - 百万级元素
TEST_F( VectorTest, LargeScaleOperations ) {
    // 大规模push_back
    wyne::vector<int> large_vec;
    for ( size_t i = 0; i < HUGE_SIZE; ++i ) {
        large_vec.push_back( i );
    }

    EXPECT_EQ( large_vec.size(), HUGE_SIZE );
    EXPECT_GE( large_vec.capacity(), HUGE_SIZE );
    EXPECT_EQ( large_vec[ 0 ], 0 );
    EXPECT_EQ( large_vec[ HUGE_SIZE / 2 ], HUGE_SIZE / 2 );
    EXPECT_EQ( large_vec[ HUGE_SIZE - 1 ], HUGE_SIZE - 1 );

    // 大规模插入
    for ( size_t i = 0; i < MEDIUM_SIZE; ++i ) {
        large_vec.insert( large_vec.begin() + i * 10, i );
    }
    EXPECT_EQ( large_vec.size(), HUGE_SIZE + MEDIUM_SIZE );

    // 大规模删除
    for ( size_t i = 0; i < MEDIUM_SIZE; ++i ) {
        large_vec.erase( large_vec.begin() );
    }
    EXPECT_EQ( large_vec.size(), HUGE_SIZE );

    // 大规模resize
    large_vec.resize( HUGE_SIZE * 2, 42 );
    EXPECT_EQ( large_vec.size(), HUGE_SIZE * 2 );
    EXPECT_EQ( large_vec[ HUGE_SIZE - 1 ], HUGE_SIZE - 1 );
    EXPECT_EQ( large_vec[ HUGE_SIZE ], 42 );
    EXPECT_EQ( large_vec[ HUGE_SIZE * 2 - 1 ], 42 );

    // 大规模clear
    large_vec.clear();
    EXPECT_TRUE( large_vec.empty() );
    EXPECT_EQ( large_vec.size(), 0 );
    EXPECT_GE( large_vec.capacity(), HUGE_SIZE * 2 );  // 容量通常不变
}

class Resource {
    int* data = nullptr;

public:
    Resource() : data( new int( 42 ) ) {}
    ~Resource() { delete data; }
    int get() const { return *data; }
    // 禁用复制以测试移动语义
    Resource( const Resource& )            = delete;
    Resource& operator=( const Resource& ) = delete;
    // 移动构造函数
    Resource( Resource&& other ) noexcept : data( other.data ) { other.data = nullptr; }
    Resource& operator=( Resource&& other ) noexcept {
        if ( this != &other ) {
            delete data;
            data       = other.data;
            other.data = nullptr;
        }
        return *this;
    }
};

std::ostream& operator<<( std::ostream& os, const Resource& w ) {
    os << w.get();
    return os;
}

// 测试复杂类型和非平凡析构函数
TEST_F( VectorTest, ComplexTypes ) {

    // 测试移动语义
    wyne::vector<Resource> resource_vec;
    resource_vec.emplace_back();
    resource_vec.emplace_back();

    EXPECT_EQ( resource_vec.size(), 2 );
    EXPECT_EQ( resource_vec[ 0 ].get(), 42 );
    EXPECT_EQ( resource_vec[ 1 ].get(), 42 );

    // 测试移动赋值
    wyne::vector<Resource> resource_move;
    resource_move = wyne::move( resource_vec );
    EXPECT_EQ( resource_move.size(), 2 );
    EXPECT_TRUE( resource_vec.empty() );

    // 测试析构函数调用
    {
        wyne::vector<Resource> scope_vec( 5 );
        EXPECT_EQ( scope_vec.size(), 5 );
        for ( auto& r : scope_vec ) {
            EXPECT_EQ( r.get(), 42 );
        }
    }  // 此处应调用所有Resource的析构函数

    // 测试异常安全 - 在复制过程中抛出异常
    struct ThrowOnCopy {
        int value;
        ThrowOnCopy( int v ) : value( v ) {}
        ThrowOnCopy( const ThrowOnCopy& other ) : value( other.value ) {
            if ( value == 42 )
                throw std::runtime_error( "Copy failed" );
        }
    };

    wyne::vector<ThrowOnCopy> throw_vec;
    throw_vec.push_back( ThrowOnCopy( 10 ) );
    throw_vec.push_back( ThrowOnCopy( 20 ) );

    try {
        throw_vec.push_back( ThrowOnCopy( 42 ) );  // 应抛出异常
        FAIL() << "Expected exception not thrown";
    }
    catch ( const std::runtime_error& ) {
        // 验证向量状态未改变
        EXPECT_EQ( throw_vec.size(), 2 );
        EXPECT_EQ( throw_vec[ 0 ].value, 10 );
        EXPECT_EQ( throw_vec[ 1 ].value, 20 );
    }
}

// 测试自定义分配器支持
TEST_F( VectorTest, CustomAllocator ) {
    struct TrackingAllocator {
        using value_type = int;
        size_t* allocation_count;

        TrackingAllocator( size_t* count ) : allocation_count( count ) {}

        int* allocate( size_t n ) {
            *allocation_count += n;
            int* temp = static_cast<int*>( ::operator new( n * sizeof( int ) ) );
            // wyne::cout << "allocate: " << temp << " n:" << n << wyne::endl;
            return temp;
        }

        void deallocate( int* p, size_t n ) {
            *allocation_count -= n;
            // wyne::cout << "deallocate: " << p << " n:" << n << wyne::endl;
            ::operator delete( p );
        }

        bool operator==( const TrackingAllocator& other ) const { return allocation_count == other.allocation_count; }
    };

    size_t            alloc_count = 0;
    TrackingAllocator alloc( &alloc_count );

    {
        wyne::vector<int, TrackingAllocator> custom_vec( alloc );
        for ( int i = 0; i < 100; ++i ) {
            custom_vec.push_back( i );
        }

        EXPECT_GT( alloc_count, 100 );
    }  // 此处vector析构

    // 验证所有内存已释放
    EXPECT_EQ( alloc_count, 0 );
}

int main( int argc, char** argv ) {
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}