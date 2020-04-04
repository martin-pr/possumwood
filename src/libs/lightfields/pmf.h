#pragma once

#include <vector>
#include <functional>

namespace lightfields {

class JointPMF;

///	a simple explicit Probability Mass Function representation
class PMF {
	public:
		/// initialises a PMF from a confidence value (0..1; 0 -> no confidence, leading to a uniform distribution; 1 -> 100% confidence, leading to a single peak of p()=1)
		static PMF fromConfidence(float c, unsigned value, unsigned size);

		/// Initialises a probability mass function as a uniform distribution of n states
		PMF(unsigned n);

		/// Returns the size of the probability array
		std::size_t size() const;

		/// Returns a probability at value
		float p(unsigned value) const;

		/// returns the value with maximum probability
		unsigned max() const;

	private:
		std::vector<float> m_p;
		float m_weight;

	friend PMF operator *(const PMF& p, const float& weight);
	friend PMF operator *(const float& weight, const PMF& p);
	friend PMF operator +(const PMF& p1, const PMF& p2);
	friend PMF operator *(const PMF& p, const JointPMF& j);
	friend PMF operator *(const JointPMF& j, const PMF& p);
};

/// an implicit representation of Joint PMF (i.e., PMF between two variables).
/// In Possumwood, currently PMFs are represented procedurally using a functor (which might potentially hold explicit values though).
class JointPMF {
	public:
		/// initialises a difference-based square joint PMF
		static JointPMF difference(unsigned size, float widthMutiplier = 1.0f);

		/// initialises a square-shaped joint PMF to a uniform distribution of size*size states
		JointPMF(unsigned size);
		/// initialises a rectangular-shaped joint PMF to a uniform distribution of rows*cols states
		JointPMF(unsigned rows, unsigned cols);

		unsigned rows() const;
		unsigned cols() const;

		/// Returns a probability at index
		float p(unsigned row, unsigned col) const;

	private:
		std::function<float(unsigned, unsigned)> m_fn;
		unsigned m_rows, m_cols;

	friend PMF operator *(const PMF& p, const float& weight);
	friend PMF operator *(const float& weight, const PMF& p);
	friend PMF operator +(const PMF& p1, const PMF& p2);
	friend PMF operator *(const PMF& p, const JointPMF& j);
	friend PMF operator *(const JointPMF& j, const PMF& p);
};

/// Weights the PMF (does not actually change the output of the PMF, just the weight that is only applicable when summing two PMFs)
PMF operator *(const PMF& p, const float& weight);
/// Weights the PMF (does not actually change the output of the PMF, just the weight that is only applicable when summing two PMFs)
PMF operator *(const float& weight, const PMF& p);
/// Weighted sum of two PMFs (change the weight using scalar multiplication; defaults to 1)
PMF operator +(const PMF& p1, const PMF& p2);

/// returns marginal probability given an input PMF (summed over columns)
PMF operator *(const PMF& p, const JointPMF& j);
/// returns marginal probability given an input PMF (summed over rows)
PMF operator *(const JointPMF& j, const PMF& p);

}
