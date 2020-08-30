#pragma once

#include "animation.h"

namespace anim {

class MotionMap;

namespace filter {

class Base : public boost::noncopyable {
  public:
	virtual ~Base() = 0;

  protected:
	virtual void init(const MotionMap& mmap);
	virtual float eval(const MotionMap& mmap, std::size_t x, std::size_t y) const = 0;

  private:
	friend class ::anim::MotionMap;
};

class LinearTransition : public Base {
  public:
	LinearTransition(const std::size_t transitionLength);

  protected:
	virtual float eval(const MotionMap& mmap, std::size_t x, std::size_t y) const override;

  private:
	int m_halfWindowWidth;
};

class IgnoreIdentity : public Base {
  public:
	IgnoreIdentity(const std::size_t transitionLength);

  protected:
	virtual float eval(const MotionMap& mmap, std::size_t x, std::size_t y) const override;

  private:
	std::size_t m_halfWindowWidth;
};

}  // namespace filter
}  // namespace anim
