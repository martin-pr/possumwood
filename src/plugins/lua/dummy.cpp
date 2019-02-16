#include <iostream>
#include <luabind/luabind.hpp>

void greet()
{
    std::cout << "hello world!\n";
}

extern "C" int init(lua_State* L)
{
    using namespace luabind;

    open(L);

    module(L)
    [
        def("greet", &greet)
    ];

    return 0;
}
