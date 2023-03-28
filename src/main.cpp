#include <exception>
#include <iostream>

int main()
{
    try
    {
        throw std::runtime_error {std::string {"asdfffff"}};
    }
    catch (const std::exception& e)
    {
        std::cout << std::addressof(e);
        std::cout << "Exception propagated to main! |" << e.what();
    }
}
