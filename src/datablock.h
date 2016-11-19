#pragma once

class Evaluator;

template<typename T>
class InAttr;

template<typename T>
class OutAttr;

class Datablock {
	public:
		template<typename T>
		const T& get(const InAttr<T>& attr);

		template<typename T>
		void set(const OutAttr<T>& attr, const T& value);

	protected:
	private:
		template<typename T>
		void set(const InAttr<T>& attr, const T& value);

		friend class Evaluator;
};
