#pragma once

#include <string>
#include <typeinfo>

namespace dependency_graph {

/// This method uses internal GCC ways to unmangle name and returns the result in a string.
const std::string unmangledName(const char* name);

/// This method uses internal GCC ways to decode the typeid.name values into the original name and re$
template<class T>
const std::string unmangledTypeId() {
        return unmangledName(typeid(T).name());
}

/// This method uses internal GCC ways to decode the typeid.name values into the original name and re$
template<class T>
const std::string unmangledTypeId(const T&) {
        return unmangledName(typeid(T).name());
}

}
