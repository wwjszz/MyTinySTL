#include "vector.h"
#include <iostream>

// struct A {
//     int a;
//     A() {}
//     A( int a ) : a( a ) {}
//     A( int a, int b ) : a( a ), A( a ) {}
// };

int main() {
    wyne::vector<int> vec( 1500 );

    std::cout << vec.capacity() << std::endl;
    std::cout << vec.size() << std::endl;
    std::cout << vec.empty() << std::endl;

    for ( int i = 0; i < 1000; ++i )
        vec.emplace_back( i );

    std::cout << vec.capacity() << std::endl;
    std::cout << vec.size() << std::endl;
    std::cout << vec.empty() << std::endl;

    // std::vector<int> test( 1, 2 );
    // std::cout << test[ 0 ] << std::endl;
    // std::cout << test.capacity() << std::endl;

    return 0;
}