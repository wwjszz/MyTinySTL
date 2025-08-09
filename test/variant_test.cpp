#include "variant.h"

int main() {
    wyne::variant<int, double, bool> v = 1;
    v.emplace<2>( 1.2 );
    return 0;
}