#include "gfx/renderer.hpp"
#include <array>
#include <format>
#include <iostream>

template<class... T>
[[nodiscard]] std::string logg(T&&... args)
{
    return std::vformat(args...);
}

int main()
{
    std::cout << logg("{}", "asdf");
    std::cout << std::format("asdf") << std::endl;
}
