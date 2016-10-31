#pragma once

#include <functional>

#include <boost/noncopyable.hpp>

class Datablock;

class Node {
	public:
		Datablock& inputs();

		const Datablock& outputs();

/*		class Attr : public boost::noncopyable {
			public:
				const std::string& name() const;

				/// forces the attribute dirty, even when no node inputs are
				void markDirty();

				/// returns true if given attribute is dirty
				bool isDirty();

			private:
				friend class Node;
		};*/

		Attr& attribute(const std::string& name);
		const Attr& attribute(const std::string& name) const;

	protected:
		Node(const std::string& name, const Metadata& meta, std::function<bool(Datablock&)> compute);
};
