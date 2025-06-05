#include "vector.h"
#include <iostream>
#include <memory>
#include <vector>

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

    return 0;
}