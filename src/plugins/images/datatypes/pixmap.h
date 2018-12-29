#pragma once

#include <actions/traits.h>

namespace possumwood {

class Pixel {
	public:
		typedef std::array<uint8_t, 3> value_t;

		Pixel(const value_t& value = value_t{{0, 0, 0}});

		const value_t& value() const;
		void setValue(const value_t& rgb);

		Pixel& operator = (const value_t& rgb);
		const value_t& operator*() const;
		operator const value_t&() const;

	private:
		value_t m_value;
};

class Pixmap {
	public:
		typedef Pixel pixel_t;
		typedef Pixel::value_t::value_type channel_t;

		Pixmap(std::size_t width = 0, std::size_t height = 0, const Pixel& defaultValue = Pixel());

		const Pixel& operator()(std::size_t x, std::size_t y) const;
		Pixel& operator()(std::size_t x, std::size_t y);

		std::size_t width() const;
		std::size_t height() const;
		bool empty() const;

	private:
		std::vector<Pixel> m_data;
		std::size_t m_width, m_height;
};

template<>
struct Traits<std::shared_ptr<const Pixmap>> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0.5, 0}};
	}
};

}
