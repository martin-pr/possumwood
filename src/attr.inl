#pragma once

#include "attr.h"

template<typename T>
InAttr<T>::InAttr() : Attr("", unsigned(-1), Attr::kInput) {
}

template<typename T>
InAttr<T>::InAttr(const std::string& name, unsigned offset) : Attr(name, offset, Attr::kInput) {
}

template<typename T>
const std::type_info& InAttr<T>::type() const {
	return typeid(T);
}

template<typename T>
std::unique_ptr<Datablock::BaseData> InAttr<T>::createData() const {
	return std::unique_ptr<Datablock::BaseData>(new Datablock::Data<T>());
}


template<typename T>
OutAttr<T>::OutAttr() : Attr("", unsigned(-1), Attr::kOutput) {
}

template<typename T>
OutAttr<T>::OutAttr(const std::string& name, unsigned offset) : Attr(name, offset, Attr::kOutput) {
}

template<typename T>
std::unique_ptr<Datablock::BaseData> OutAttr<T>::createData() const {
	return std::unique_ptr<Datablock::BaseData>(new Datablock::Data<T>());
}

template<typename T>
const std::type_info& OutAttr<T>::type() const {
	return typeid(T);
}
