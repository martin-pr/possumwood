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
		void addAttribute(InAttribute<T>& in);

		/// registers an output attribute.
		/// Each attribute instance should be held statically in the
		/// implementation of the "node" concept of the target application.
		/// This call does *not* take ownership of the attribute, and assumes
		/// that it will be available throughout the application run.
		template<typename T>
		void addAttribute(OutAttribute<T>& out);

		template<typename T, typename U>
		void addInfluence(const InAttribute<T>& in, const OutAttribute<U>& out);
};
