#pragma once

#include <vector>
#include <cstdint>
#include <atomic>

namespace lightfields {

/// An implementation of a simple parallel-safe bitfield
class Bitfield {
	public:
		class Accessor {
			public:
				operator bool() const;
				Accessor& operator = (bool val);

			private:
				Accessor(Bitfield* parent, std::size_t index, std::size_t offset);

				Bitfield* m_parent;
				std::size_t m_index, m_offset;

			friend class Bitfield;
		};

		Bitfield(std::size_t size);

		std::size_t size() const;
		bool empty() const;

		Accessor operator[](std::size_t index);
		bool operator[](std::size_t index) const;

	private:
		std::size_t m_size;
		std::vector<std::atomic<uint32_t>> m_data;
};

}
