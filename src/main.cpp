#include <exception>
#include <iostream>
#include "vengine.hpp"

int main(int argc, char** argv) {
    try {
        VEngine engine;
        engine.Run();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}