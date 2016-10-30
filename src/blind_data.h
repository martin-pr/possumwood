#pragma once

class BlindData {
	public:
		template<typename T>
		BlindData(const T& value);

		template<typename T>
		const T& get() const;

		template<typename T>
		T& get();
};
