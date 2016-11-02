#pragma once

template<typename T>
class InAttribute;

template<typename T>
class OutAttribute;

class Metadata {
	public:
		Metadata(const std::string& nodeType);

		/// registers an input attribute.
		/// Each attribute instance should be held statically in the
		/// implementation of the "node" concept of the target application.
		/// This call does *not* take ownership of the attribute, and assumes
		/// that it will be available throughout the application run.
		template<typename T>
		void addAttribute(InAttribute<T>& in, const std::string& name);

		/// registers an output attribute.
		/// Each attribute instance should be held statically in the
		/// implementation of the "node" concept of the target application.
		/// This call does *not* take ownership of the attribute, and assumes
		/// that it will be available throughout the application run.
		template<typename T>
		void addAttribute(OutAttribute<T>& out, const std::string& name);

		template<typename T, typename U>
		void addInfluence(const InAttribute<T>& in, const OutAttribute<U>& out);

		/// compute method of this node
		void setCompute(std::function<bool(Datablock&)> compute);

		/// returns the number of attributes currently present
		size_t attributeCount() const;

		/// returns an attribute reference
		Attribute& attr(unsigned index);
		/// returns an attribute reference
		const Attribute& attr(unsigned index) const;

};
