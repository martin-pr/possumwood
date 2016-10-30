#pragma once

template<typename T>
class InAttribute;

template<typename T>
class OutAttribute;

class Metadata {
	public:
		Metadata(const std::string& nodeType);

		template<typename T>
		void addAttribute(InAttribute<T>& in);

		template<typename T>
		void addAttribute(OutAttribute<T>& out);

		template<typename T, typename U>
		void addInfluence(const InAttribute<T>& in, const OutAttribute<U>& out);
};
