#pragma once

#include <array>
#include <vector>

namespace anim {

class Polygons {
  public:
	std::array<std::size_t, 3>& add(std::size_t p1, std::size_t p2, std::size_t p3);

	std::array<std::size_t, 3>& operator[](std::size_t index);
	const std::array<std::size_t, 3>& operator[](std::size_t index) const;

	bool empty() const;
	std::size_t size() const;

	typedef std::vector<std::array<std::size_t, 3>>::const_iterator const_iterator;
	const_iterator begin() const;
	const_iterator end() const;

	typedef std::vector<std::array<std::size_t, 3>>::iterator iterator;
	iterator begin();
	iterator end();

  private:
	std::vector<std::array<std::size_t, 3>> m_polygons;
};
}  // namespace anim
