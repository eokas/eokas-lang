#include <iostream>
#include <array>
#include <initializer_list>

struct func_0 {
    virtual auto operator()(int a) const -> int = 0;
};

void module_main()
{
    const auto list = std::array{ 1, 2, 3 };
    std::cout << (list[0]);
}
