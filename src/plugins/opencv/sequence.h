#pragma once

#include <memory>
#include <vector>

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/filesystem/path.hpp>

#include "frame.h"

namespace possumwood { namespace opencv {

class Sequence final {
	public:
		class Item {
			public:
				class Meta {
					public:
						bool empty() const;

						float operator[](const std::string& key) const;
						float& operator[](const std::string& key);

						typedef std::map<std::string, float>::const_iterator const_iterator;
						const_iterator begin() const;
						const_iterator end() const;

						static Meta merge(const Meta& m1, const Meta& m2);

					private:
						std::map<std::string, float> m_meta;
				};

				Item();
				Item(const cv::Mat& m, const Meta& meta = Meta());

				cv::Mat& operator*() { return m_mat; }
				const cv::Mat& operator*() const { return m_mat; }

				cv::Mat* operator->() { return &m_mat; }
				const cv::Mat* operator->() const { return &m_mat; }

				Meta& meta();
				const Meta& meta() const;

				bool operator == (const Item& i) const;
				bool operator != (const Item& i) const;

			private:
				cv::Mat m_mat;
				Meta m_meta;
		};

		Sequence(std::size_t size = 0);

		Sequence clone() const;

		Item& add(const cv::Mat& frame, const Item::Meta& meta = Item::Meta());

		bool isValid() const;

		bool empty() const;
		std::size_t size() const;

		int type() const;
		int rows() const;
		int cols() const;
		int depth() const;
		int channels() const;

		Item& operator[](std::size_t index);
		const Item& operator[](std::size_t index) const;

		typedef std::vector<Item>::iterator iterator;
		iterator begin();
		iterator end();

		typedef std::vector<Item>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		const Item& front() const;
		const Item& back() const;

		bool operator == (const Sequence& f) const;
		bool operator != (const Sequence& f) const;

	private:
		std::vector<Item> m_sequence;
};

std::ostream& operator << (std::ostream& out, const Sequence& f);

}

template<>
struct Traits<opencv::Sequence> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0, 0}};
	}
};

}
