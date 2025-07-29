#include "optional.h"
#include <compare>
#include <gtest/gtest.h>
#include <string>
#include <utility>
#include <vector>

using wyne::none;
using wyne::optional;

// 用于测试构造/析构/赋值等
struct Counter {
    static int constructed;
    static int destructed;
    static int copied;
    static int moved;
    int        value;

    Counter( int v = 0 ) : value( v ) { ++constructed; }
    Counter( const Counter& o ) : value( o.value ) {
        ++constructed;
        ++copied;
    }
    Counter( Counter&& o ) noexcept : value( o.value ) {
        ++constructed;
        ++moved;
    }
    Counter& operator=( const Counter& o ) {
        value = o.value;
        ++copied;
        return *this;
    }
    Counter& operator=( Counter&& o ) noexcept {
        value = o.value;
        ++moved;
        return *this;
    }
    ~Counter() { ++destructed; }

    bool operator==( const Counter& o ) const { return value == o.value; }
    auto operator<=>( const Counter& o ) const = default;

    static void reset() { constructed = destructed = copied = moved = 0; }
};
int Counter::constructed = 0;
int Counter::destructed  = 0;
int Counter::copied      = 0;
int Counter::moved       = 0;

TEST( OptionalTest, DefaultConstruct ) {
    optional<int> o;
    EXPECT_FALSE( o.has_value() );
    EXPECT_FALSE( static_cast<bool>( o ) );
}

TEST( OptionalTest, NoneConstruct ) {
    optional<int> o( none );
    EXPECT_FALSE( o.has_value() );
}

TEST( OptionalTest, ValueConstruct ) {
    optional<int> o( 42 );
    EXPECT_TRUE( o.has_value() );
    EXPECT_EQ( o.value(), 42 );
    EXPECT_EQ( *o, 42 );
}

TEST( OptionalTest, CopyConstruct ) {
    optional<int> o1( 123 );
    optional<int> o2( o1 );
    EXPECT_TRUE( o2.has_value() );
    EXPECT_EQ( o2.value(), 123 );

    optional<int> o3;
    optional<int> o4( o3 );
    EXPECT_FALSE( o4.has_value() );
}

TEST( OptionalTest, MoveConstruct ) {
    optional<std::string> o1( std::string( "hello" ) );
    optional<std::string> o2( std::move( o1 ) );
    EXPECT_TRUE( o2.has_value() );
    EXPECT_EQ( o2.value(), "hello" );
}

TEST( OptionalTest, InPlaceConstruct ) {
    optional<std::vector<int>> o( std::in_place, 3, 7 );
    EXPECT_TRUE( o.has_value() );
    EXPECT_EQ( o->size(), 3 );
    EXPECT_EQ( ( *o )[ 0 ], 7 );
}

TEST( OptionalTest, InPlaceInitListConstruct ) {
    optional<std::vector<int>> o( std::in_place, { 1, 2, 3 } );
    EXPECT_TRUE( o.has_value() );
    EXPECT_EQ( o->size(), 3 );
    EXPECT_EQ( ( *o )[ 2 ], 3 );
}

TEST( OptionalTest, AssignNone ) {
    optional<int> o( 5 );
    o = none;
    EXPECT_FALSE( o.has_value() );
}

TEST( OptionalTest, AssignValue ) {
    optional<int> o;
    o = 99;
    EXPECT_TRUE( o.has_value() );
    EXPECT_EQ( o.value(), 99 );
}

TEST( OptionalTest, AssignOptional ) {
    optional<int> o1( 7 );
    optional<int> o2;
    o2 = o1;
    EXPECT_TRUE( o2.has_value() );
    EXPECT_EQ( o2.value(), 7 );

    optional<int> o3;
    o2 = o3;
    EXPECT_FALSE( o2.has_value() );
}

TEST( OptionalTest, AssignOptionalMove ) {
    optional<std::string> o1( "abc" );
    optional<std::string> o2;
    o2 = std::move( o1 );
    EXPECT_TRUE( o2.has_value() );
    EXPECT_EQ( o2.value(), "abc" );
}

TEST( OptionalTest, Emplace ) {
    optional<std::string> o;
    auto&                 ref = o.emplace( 5, 'x' );
    EXPECT_TRUE( o.has_value() );
    EXPECT_EQ( o.value(), "xxxxx" );
    EXPECT_EQ( &ref, &o.value() );
}

TEST( OptionalTest, EmplaceInitList ) {
    optional<std::vector<int>> o;
    o.emplace( { 1, 2, 3, 4 } );
    EXPECT_TRUE( o.has_value() );
    EXPECT_EQ( o->size(), 4 );
}

TEST( OptionalTest, Reset ) {
    optional<int> o( 1 );
    o.reset();
    EXPECT_FALSE( o.has_value() );
}

TEST( OptionalTest, Swap ) {
    optional<int> o1( 1 ), o2( 2 );
    o1.swap( o2 );
    EXPECT_EQ( o1.value(), 2 );
    EXPECT_EQ( o2.value(), 1 );

    optional<int> o3( 3 ), o4;
    o3.swap( o4 );
    EXPECT_FALSE( o3.has_value() );
    EXPECT_TRUE( o4.has_value() );
    EXPECT_EQ( o4.value(), 3 );
}

TEST( OptionalTest, ValueOr ) {
    optional<int> o1( 5 ), o2;
    EXPECT_EQ( o1.value_or( 10 ), 5 );
    EXPECT_EQ( o2.value_or( 10 ), 10 );
}

TEST( OptionalTest, ValueThrows ) {
    optional<int> o;
    EXPECT_THROW( o.value(), wyne::optional_empty_exception );
}

TEST( OptionalTest, OperatorBool ) {
    optional<int> o;
    if ( o ) {
        FAIL() << "Should not enter";
    }
    o = 1;
    if ( !o ) {
        FAIL() << "Should enter";
    }
}

TEST( OptionalTest, OperatorArrow ) {
    optional<std::string> o( "hello" );
    EXPECT_EQ( o->size(), 5 );
}

TEST( OptionalTest, OperatorEqual ) {
    optional<int> o1( 1 ), o2( 1 ), o3( 2 ), o4;
    EXPECT_TRUE( o1 == o2 );
    EXPECT_FALSE( o1 == o3 );
    EXPECT_FALSE( o1 == o4 );
    EXPECT_TRUE( o4 == optional<int>() );
}

TEST( OptionalTest, OperatorSpaceship ) {
    optional<int> o1( 1 ), o2( 2 ), o3;
    EXPECT_TRUE( ( o1 < o2 ) );
    EXPECT_TRUE( ( o3 < o1 ) );
    EXPECT_TRUE( ( o3 == optional<int>() ) );
    EXPECT_TRUE( ( o2 > o1 ) );
}

TEST( OptionalTest, CounterLifeCycle ) {
    Counter::reset();
    {
        optional<Counter> o1;
        EXPECT_EQ( Counter::constructed, 0 );
        o1 = Counter( 42 );
        EXPECT_EQ( Counter::constructed, 2 );  // one temp, one in optional
        EXPECT_EQ( Counter::copied + Counter::moved, 1 );
        o1.reset();
        EXPECT_EQ( Counter::destructed, 2 );  // optional's value destroyed
    }
    EXPECT_EQ( Counter::destructed, Counter::constructed );
}

TEST( OptionalTest, MakeOptional ) {
    auto o1 = wyne::make_optional( 123 );
    EXPECT_TRUE( o1.has_value() );
    EXPECT_EQ( o1.value(), 123 );

    auto o2 = wyne::make_optional<std::string>( 5, 'a' );
    EXPECT_EQ( o2.value(), "aaaaa" );

    auto o3 = wyne::make_optional<std::vector<int>>( { 1, 2, 3 } );
    EXPECT_EQ( o3->size(), 3 );
}

TEST( OptionalTest, GetPointer ) {
    optional<int> o1( 5 ), o2;
    EXPECT_NE( o1.get_pointer(), nullptr );
    EXPECT_EQ( o2.get_pointer(), nullptr );
}

TEST( OptionalTest, ConstCorrectness ) {
    const optional<int> o1( 7 ), o2;
    EXPECT_EQ( o1.value_or( 9 ), 7 );
    EXPECT_EQ( o2.value_or( 9 ), 9 );
    EXPECT_EQ( *o1, 7 );
    EXPECT_EQ( o1.get_pointer() != nullptr, true );
    EXPECT_EQ( o2.get_pointer(), nullptr );
}

TEST( OptionalCompareTest, OptionalAndValueEquality ) {
    optional<int> o1( 5 ), o2;
    EXPECT_TRUE( o1 == 5 );
    EXPECT_FALSE( o1 == 6 );
    EXPECT_FALSE( o2 == 5 );
    EXPECT_TRUE( o2 != 5 );
    EXPECT_TRUE( o1 != 6 );
    EXPECT_FALSE( o1 != 5 );

    // 反向
    EXPECT_TRUE( 5 == o1 );
    EXPECT_FALSE( 6 == o1 );
    EXPECT_TRUE( 5 != o2 );
    EXPECT_TRUE( 6 != o1 );
    EXPECT_FALSE( 5 != o1 );
}

TEST( OptionalCompareTest, OptionalAndValueRelational ) {
    optional<int> o1( 5 ), o2;
    EXPECT_TRUE( o1 < 6 );
    EXPECT_FALSE( o1 < 5 );
    EXPECT_TRUE( o1 <= 5 );
    EXPECT_FALSE( o1 > 5 );
    EXPECT_TRUE( o1 > 4 );
    EXPECT_TRUE( o1 >= 5 );

    // 空 optional 总是小于任何 value
    EXPECT_TRUE( o2 < 5 );
    EXPECT_FALSE( o2 > 5 );
    EXPECT_TRUE( o2 <= 5 );
    EXPECT_FALSE( o2 >= 5 );

    // 反向
    EXPECT_TRUE( 6 > o1 );
    EXPECT_FALSE( 5 > o1 );
    EXPECT_TRUE( 5 >= o1 );
    EXPECT_FALSE( 4 >= o1 );
    EXPECT_TRUE( 4 < o1 );
    EXPECT_TRUE( 5 <= o1 );

    // 空 optional 总是小于任何 value
    EXPECT_TRUE( 5 > o2 );
    EXPECT_FALSE( 5 < o2 );
    EXPECT_TRUE( 5 >= o2 );
    EXPECT_FALSE( 5 <= o2 );
}

TEST( OptionalCompareTest, OptionalAndNoneEquality ) {
    optional<int> o1( 5 ), o2;
    EXPECT_FALSE( o1 == none );
    EXPECT_TRUE( o2 == none );
    EXPECT_TRUE( o1 != none );
    EXPECT_FALSE( o2 != none );

    // 反向
    EXPECT_FALSE( none == o1 );
    EXPECT_TRUE( none == o2 );
    EXPECT_TRUE( none != o1 );
    EXPECT_FALSE( none != o2 );
}

TEST( OptionalCompareTest, OptionalAndNoneRelational ) {
    optional<int> o1( 5 ), o2;
    // none < any value
    EXPECT_TRUE( o2 < o1 );
    EXPECT_FALSE( o1 < o2 );
    EXPECT_TRUE( o2 <= o1 );
    EXPECT_FALSE( o1 <= o2 );
    EXPECT_TRUE( o1 > o2 );
    EXPECT_FALSE( o2 > o1 );
    EXPECT_TRUE( o1 >= o2 );
    EXPECT_FALSE( o2 >= o1 );

    // optional < none: always false
    EXPECT_FALSE( o1 < none );
    EXPECT_TRUE( o2 < none == false );
    EXPECT_TRUE( o1 > none );
    EXPECT_FALSE( o2 > none );

    // none < optional: true if optional has value
    EXPECT_TRUE( none < o1 );
    EXPECT_FALSE( none < o2 );
    EXPECT_FALSE( none > o1 );
    EXPECT_FALSE( none > o2 );
}

TEST( OptionalCompareTest, OptionalStringAndValue ) {
    optional<std::string> o1( "abc" ), o2;
    EXPECT_TRUE( o1 == std::string( "abc" ) );
    EXPECT_FALSE( o1 == std::string( "def" ) );
    EXPECT_TRUE( o1 < std::string( "bcd" ) );
    EXPECT_TRUE( o2 < std::string( "zzz" ) );
    EXPECT_TRUE( std::string( "zzz" ) > o1 );
    EXPECT_TRUE( std::string( "abc" ) == o1 );
    EXPECT_TRUE( std::string( "aaa" ) < o1 );
    EXPECT_TRUE( o2 == none );
    EXPECT_TRUE( none == o2 );
}
