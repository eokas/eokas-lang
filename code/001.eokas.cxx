#include <iostream>
#include <array>
#include <initializer_list>

void module_main()
{
    const auto list = std::array{ 1, 2, 3 };
    std::cout << (list[0]);
}
