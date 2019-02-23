#pragma once

#include <actions/traits.h>

namespace possumwood {

template<typename BASE>
class Pixel {
	public:
		typedef std::array<BASE, 3> value_t;

		Pixel(const value_t& value = value_t{{0, 0, 0}});

		const value_t& value() const;
		void setValue(const value_t& rgb);

		Pixel& operator = (const value_t& rgb);
		const value_t& operator*() const;
		operator const value_t&() const;

	private:
		value_t m_value;
};

// pixel types (explicit template instantiation only!)
typedef Pixel<uint8_t> LDRPixel;
typedef Pixel<float> HDRPixel;

template<typename PIXEL>
class Pixmap {
	public:
		typedef PIXEL pixel_t;
		typedef typename PIXEL::value_t::value_type channel_t;

		Pixmap(std::size_t width = 0, std::size_t height = 0, const pixel_t& defaultValue = pixel_t());

		const pixel_t& operator()(std::size_t x, std::size_t y) const;
		pixel_t& operator()(std::size_t x, std::size_t y);

		std::size_t width() const;
		std::size_t height() const;
		bool empty() const;

	private:
		std::vector<pixel_t> m_data;
		std::size_t m_width, m_height;
};

// pixmap types (explicit template instantiation only!)
typedef Pixmap<Pixel<uint8_t>> LDRPixmap;
typedef Pixmap<Pixel<float>> HDRPixmap;


template<>
struct Traits<std::shared_ptr<const LDRPixmap>> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0.5, 0}};
	}
};

template<>
struct Traits<std::shared_ptr<const HDRPixmap>> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0.75, 0.5}};
	}
};

}
