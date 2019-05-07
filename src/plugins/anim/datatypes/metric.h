#pragma once

#include <boost/noncopyable.hpp>

#include "animation.h"

namespace anim {

class MotionMap;

namespace metric {

class Base : public boost::noncopyable {
	public:
		virtual ~Base() = 0;

	protected:
		virtual float eval(const Animation& a1, std::size_t f1, const Animation& a2, std::size_t f2) const = 0;

	friend class ::anim::MotionMap;
};

class LocalAngle : public Base {
	public:
		virtual ~LocalAngle();

	protected:
		virtual float eval(const Animation& a1, std::size_t f1, const Animation& a2, std::size_t f2) const override;
};

} }
