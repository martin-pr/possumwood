#pragma once

#include "attr.h"

namespace dependency_graph {

template<typename T>
TypedAttr<T>::TypedAttr(const std::string& name, unsigned offset, Category cat) : Attr(name, offset, cat) {
}

template<typename T>
const std::type_info& TypedAttr<T>::type() const {
	return typeid(T);
}

template<typename T>
std::unique_ptr<Datablock::BaseData> TypedAttr<T>::createData() const {
	return std::unique_ptr<Datablock::BaseData>(new Datablock::Data<T>());
}


template<typename T>
InAttr<T>::InAttr() : TypedAttr<T>("", unsigned(-1), Attr::kInput) {
}

template<typename T>
InAttr<T>::InAttr(const std::string& name, unsigned offset) : TypedAttr<T>(name, offset, Attr::kInput) {
}


template<typename T>
OutAttr<T>::OutAttr() : TypedAttr<T>("", unsigned(-1), Attr::kOutput) {
}

template<typename T>
OutAttr<T>::OutAttr(const std::string& name, unsigned offset) : TypedAttr<T>(name, offset, Attr::kOutput) {
}

}
