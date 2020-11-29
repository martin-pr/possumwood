#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include <boost/filesystem/path.hpp>
#include <boost/iterator/indirect_iterator.hpp>

#include <OpenEXR/ImathVec.h>

#include "frame.h"

namespace possumwood {
namespace opencv {

/// Sequence is just a container for cv::Mats (similar to Frame), where all items have consistent
/// metadata (size, type, color depth). First added image determines the metadata for the sequence;
/// all images need to be consistent (or Sequence throws a runtime_error when adding an image).
/// Please be aware that cv::Mat implementation is essentially a NON CONST SHARED POINTER, and it is
/// fundamentally flawed - COPY DOESN'T DUPLICATE THE INTERNAL DATA! There is no good way to solve
/// this without modifications to OpenCV. The sequence class therefore BEHACES THE SAME WAY!
/// Before any modifications, please CLONE IT FIRST! (I hate this so much, but can't fix it.)
/// Ref: https://stackoverflow.com/questions/13713625/is-cvmat-class-flawed-by-design
class Sequence final {
  public:
	struct Metadata {
		Metadata();
		Metadata(int type, int rows, int cols);

		int type = 0;      // OpenCV type, e.g., CV_8UC1 or CV_32FC3
		int rows = 0;      // number of rows of all images in this sequence
		int cols = 0;      // number of columns of all images in this sequence
		int depth = 0;     // OpenCV depth, e.g., CV_8U or CV_32F
		int channels = 0;  // number of channels (e.g., 1 for mono, 3 for rgb)

		bool operator==(const Metadata& meta) const;
		bool operator!=(const Metadata& meta) const;
	};

	// behaves essentially like an iterator
	class MatProxy {
	  public:
		MatProxy& operator=(cv::Mat&& m);
		operator const cv::Mat &() const;

	  private:
		MatProxy(const Imath::V2i& index, cv::Mat* m, Sequence* s);

		Imath::V2i m_index;
		cv::Mat* m_mat;
		Sequence* m_seq;

		friend class Sequence;
	};

	Sequence(const Metadata& meta = Metadata());

	Sequence(const Sequence& s);
	Sequence& operator=(const Sequence& s);

	bool empty() const;
	bool hasOneElement() const;

	const Imath::V2i& min() const;
	const Imath::V2i& max() const;
	const Metadata& meta() const;

	// a bad attempt at semi const correctness - doesn't work, because cv::Mat doesn't.
	// Please don't copy a Mat + change, it would break everything.
	const cv::Mat& operator[](const Imath::V2i& index) const;
	MatProxy operator[](const Imath::V2i& index);

	const cv::Mat& operator()(int x, int y) const;
	MatProxy operator()(int x, int y);

	typedef std::map<Imath::V2i, cv::Mat>::iterator iterator;
	iterator begin();
	iterator end();

	typedef std::map<Imath::V2i, cv::Mat>::const_iterator const_iterator;
	const_iterator find(const Imath::V2i& index) const;
	const_iterator begin() const;
	const_iterator end() const;

	bool operator==(const Sequence& f) const;
	bool operator!=(const Sequence& f) const;

	static bool hasMatchingKeys(const Sequence& s1, const Sequence& s2);

  private:
	void updateMeta(const Imath::V2i& index, const cv::Mat& m);

	struct Comparator {
		bool operator()(const Imath::V2i& v1, const Imath::V2i& v2) const;
	};

	std::map<Imath::V2i, cv::Mat, Comparator> m_sequence;
	Metadata m_meta;

	Imath::V2i m_min, m_max;

	mutable std::mutex m_mutex;

	friend std::ostream& operator<<(std::ostream& out, const Sequence& f);
};

std::ostream& operator<<(std::ostream& out, const Sequence& f);

}  // namespace opencv

template <>
struct Traits<opencv::Sequence> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0, 0}};
	}
};

}  // namespace possumwood
