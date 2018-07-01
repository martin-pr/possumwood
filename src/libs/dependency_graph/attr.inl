#pragma once

#include "attr.h"

namespace dependency_graph {

template <typename T>
TypedAttr<T>::TypedAttr(const std::string& name, Category cat, const T& defaultValue)
    : Attr(name, cat, Data<T>(defaultValue)) {
}

template <typename T>
InAttr<T>::InAttr() : TypedAttr<T>("", Attr::kInput, T()) {
}

template <typename T>
InAttr<T>::InAttr(const std::string& name, const T& defaultValue)
    : TypedAttr<T>(name, Attr::kInput, defaultValue) {
}

template <typename T>
OutAttr<T>::OutAttr() : TypedAttr<T>("", Attr::kOutput, T()) {
}

template <typename T>
OutAttr<T>::OutAttr(const std::string& name, const T& defaultValue)
    : TypedAttr<T>(name, Attr::kOutput, defaultValue) {
}
}
