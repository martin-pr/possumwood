#pragma once

#include <boost/noncopyable.hpp>

#include <functional>
#include <typeindex>

#include <dependency_graph/rtti.h>
#include <dependency_graph/data.inl>
#include <dependency_graph/io.h>

#include "io/json.h"

namespace possumwood {

/// IO base class, registering all existing IO instances / specialisations.
/// Facilitates loading and saving of dependency_graph::Data<T> instances in
/// plugins.
class IOBase : public boost::noncopyable {
	public:
		typedef std::function<void(possumwood::io::json&, const dependency_graph::BaseData&)> to_fn;
		typedef std::function<void(const possumwood::io::json&, dependency_graph::BaseData&)> from_fn;

		IOBase(const std::type_index& type, to_fn toJson, from_fn fromJson);
		virtual ~IOBase();

	private:
		std::type_index m_type;
};

template<typename T>
class IO : public IOBase {
	public:
		typedef std::function<void(possumwood::io::json&, const T&)> to_fn;
		typedef std::function<void(const possumwood::io::json&, T&)> from_fn;

		IO(to_fn toJson, from_fn fromJson) : IOBase(
			typeid(T),
			[toJson](possumwood::io::json& json, const dependency_graph::BaseData& data) {
				const dependency_graph::TypedData<T>& typed = dynamic_cast<const dependency_graph::TypedData<T>&>(data);
				toJson(json, typed.get());
			},
			[fromJson](const possumwood::io::json& json, dependency_graph::BaseData& data) {
				dependency_graph::TypedData<T>& typed = dynamic_cast<dependency_graph::TypedData<T>&>(data);

				T val = typed.get();
				fromJson(json, val);

				typed.set(val);
			}
		) {}
};

namespace io {

void fromJson(const json& j, dependency_graph::BaseData& data);
void toJson(json& j, const dependency_graph::BaseData& data);

} }
