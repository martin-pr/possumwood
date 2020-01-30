#pragma once

#include "attr.h"
#include "data.inl"

namespace dependency_graph {

template <typename T>
TypedAttr<T>::TypedAttr(const std::string& name, Category cat, T&& defaultValue, unsigned flags)
    : Attr(name, cat, Data(std::move(defaultValue)), flags) {
}

template <typename T>
InAttr<T>::InAttr() : TypedAttr<T>("", Attr::kInput, T(), 0) {
}

template <typename T>
InAttr<T>::InAttr(const std::string& name, T&& defaultValue, unsigned flags)
    : TypedAttr<T>(name, Attr::kInput, std::move(defaultValue), flags) {
}

template <typename T>
OutAttr<T>::OutAttr() : TypedAttr<T>("", Attr::kOutput, T(), 0) {
}

template <typename T>
OutAttr<T>::OutAttr(const std::string& name, T&& defaultValue, unsigned flags)
    : TypedAttr<T>(name, Attr::kOutput, std::move(defaultValue), flags) {
}
}
