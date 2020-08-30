#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "vec2.h"

namespace lightfields {

/// Neighbourhood functor evaluation
class Neighbours {
  public:
	enum Type { k4, k8, k8Weighted };

	Neighbours(const V2i& size);
	virtual ~Neighbours();

	virtual void eval(const V2i& pos, const std::function<void(const V2i&, float)>& fn) const = 0;

	/// Factory enumerator
	static const std::vector<std::pair<std::string, Type>>& types();

	/// Factory method
	static std::unique_ptr<Neighbours> create(Type t, const V2i& size);

  protected:
	const V2i& size() const;

  private:
	V2i m_size;
};

}  // namespace lightfields
