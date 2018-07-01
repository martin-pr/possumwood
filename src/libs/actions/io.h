#pragma once

#include <boost/noncopyable.hpp>

#include <functional>

#include <dependency_graph/rtti.h>
#include <dependency_graph/data.inl>
#include <dependency_graph/io.h>

namespace possumwood {

/// IO base class, registering all existing IO instances / specialisations.
/// Facilitates loading and saving of dependency_graph::Data<T> instances in
/// plugins.
class IOBase : public boost::noncopyable {
	public:
		typedef std::function<void(dependency_graph::io::json&, const dependency_graph::BaseData&)> to_fn;
		typedef std::function<void(const dependency_graph::io::json&, dependency_graph::BaseData&)> from_fn;

		IOBase(const std::string& type, to_fn toJson, from_fn fromJson);
		virtual ~IOBase();

	private:
		std::string m_type;
};

template<typename T>
class IO : public IOBase {
	public:
		typedef std::function<void(dependency_graph::io::json&, const T&)> to_fn;
		typedef std::function<void(const dependency_graph::io::json&, T&)> from_fn;

		IO(to_fn toJson, from_fn fromJson) : IOBase(
			dependency_graph::unmangledTypeId<T>(),
			[toJson](dependency_graph::io::json& json, const dependency_graph::BaseData& data) {
				const dependency_graph::Data<T>& typed = dynamic_cast<const dependency_graph::Data<T>&>(data);
				toJson(json, typed.value);
			},
			[fromJson](const dependency_graph::io::json& json, dependency_graph::BaseData& data) {
				dependency_graph::Data<T>& typed = dynamic_cast<dependency_graph::Data<T>&>(data);
				fromJson(json, typed.value);
			}
		) {}
};

}
