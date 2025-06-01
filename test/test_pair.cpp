#include "../wyne_stl/util.h"
#include <iostream>
#include <string>
#include <vector>

using namespace wyne;

// 简单的日志宏（可选）
#define LOG( msg ) std::cout << "[LOG] " << msg << std::endl

void test_default_constructor() {
    pair<int, std::string> p;
    LOG( "Default constructor: (" << p.first << ", " << p.second << ")" );
}

void test_copy_constructor() {
    pair<int, std::string> p1( 42, "hello" );
    pair<int, std::string> p2 = p1;
    LOG( "Copy constructor: (" << p2.first << ", " << p2.second << ")" );
}

void test_move_constructor() {
    pair<std::string, int> p1( "move", 100 );
    pair<std::string, int> p2 = std::move( p1 );
    LOG( "Move constructor: (" << p2.first << ", " << p2.second << ")" );
}

void test_explicit_constructor() {
    pair<int, double> p( 3, 3.14 );
    LOG( "Explicit constructor: (" << p.first << ", " << p.second << ")" );
}

void test_template_constructor() {
    pair<long, const char*> p1( 123L, "world" );
    pair<int, std::string>  p2( p1 );  // implicit conversion allowed
    LOG( "Template constructor: (" << p2.first << ", " << p2.second << ")" );
}

void test_move_assignment() {
    pair<std::string, int> p1( "old", 0 );
    pair<std::string, int> p2( "new", 99 );
    p1 = std::move( p2 );
    LOG( "Move assignment: (" << p1.first << ", " << p1.second << ")" );
}

void test_cross_type_assignment() {
    pair<long, const char*> p1( 100L, "assign" );
    pair<int, std::string>  p2;
    p2 = p1;
    LOG( "Cross-type assignment: (" << p2.first << ", " << p2.second << ")" );
}

void test_cross_type_move_assignment() {
    pair<long, const char*> p1( 200L, "move_assign" );
    pair<int, std::string>  p2;
    p2 = std::move( p1 );
    LOG( "Cross-type move assignment: (" << p2.first << ", " << p2.second << ")" );
}

void test_comparison_operators() {
    pair<int, std::string> p1( 1, "a" );
    pair<int, std::string> p2( 1, "b" );
    pair<int, std::string> p3( 2, "a" );

    LOG( "p1 == p2? " << ( p1 == p2 ? "yes" : "no" ) );
    LOG( "p1 != p2? " << ( p1 != p2 ? "yes" : "no" ) );
    LOG( "p1 < p2? " << ( p1 < p2 ? "yes" : "no" ) );
    LOG( "p1 > p3? " << ( p1 > p3 ? "yes" : "no" ) );
    LOG( "p1 <= p2? " << ( p1 <= p2 ? "yes" : "no" ) );
    LOG( "p1 >= p3? " << ( p1 >= p3 ? "yes" : "no" ) );
}

void test_swap() {
    pair<int, std::string> p1( 1, "apple" );
    pair<int, std::string> p2( 2, "banana" );

    LOG( "Before swap: p1=(" << p1.first << "," << p1.second << "), p2=(" << p2.first << "," << p2.second << ")" );
    p1.swap( p2 );
    LOG( "After swap: p1=(" << p1.first << "," << p1.second << "), p2=(" << p2.first << "," << p2.second << ")" );
}

void test_make_pair() {
    auto p = wyne::make_pair( 3.14, std::string( "pi" ) );
    LOG( "make_pair: (" << p.first << ", " << p.second << ")" );
}

void test_usage_in_vector() {
    std::vector<pair<int, std::string>> vec;
    vec.push_back( make_pair( 1, "one" ) );
    vec.emplace_back( 2, "two" );

    for ( const auto& p : vec ) {
        LOG( "Vector element: (" << p.first << ", " << p.second << ")" );
    }
}

int main() {
    LOG( "=== Testing wyne::pair ===" );
    test_default_constructor();
    test_copy_constructor();
    test_move_constructor();
    test_explicit_constructor();
    test_template_constructor();
    test_move_assignment();
    test_cross_type_assignment();
    test_cross_type_move_assignment();
    test_comparison_operators();
    test_swap();
    test_make_pair();
    test_usage_in_vector();
    LOG( "=== All tests passed! ===" );
    return 0;
}