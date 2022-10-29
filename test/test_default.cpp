#include <craftworld/craftworld.h>

#include <iostream>

using namespace craftworld;

void test_default() {
    GameParameters params = kDefaultGameParams;
    CraftWorldGameState state(params);
    std::cout << state << std::endl;
    std::cout << state.is_solution() << std::endl;
}

int main() {
    test_default();
}