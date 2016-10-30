#pragma once

class Evaluator;

template<typename T>
class InAttribute;

template<typename T>
class OutAttribute;

class Datablock {
	public:
		template<typename T>
		const T& get(const InAttribute<T>& attr);

		template<typename T>
		void set(const OutAttribute<T>& attr, const T& value);

	protected:
		friend class Evaluator;
};
