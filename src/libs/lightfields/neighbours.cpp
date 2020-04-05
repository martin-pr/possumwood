#include "neighbours.h"

#include <cassert>

namespace lightfields {

Neighbours::Neighbours(const V2i& size) : m_size(size) {
}

Neighbours::~Neighbours() {
}

const V2i& Neighbours::size() const {
	return m_size;
}

std::map<std::string, Neighbours::Type> Neighbours::types() {
	return std::map<std::string, Type>{
		std::pair<std::string, Type>("4-neighbourhood", k4),
		std::pair<std::string, Type>("8-neighbourhood", k8),
		std::pair<std::string, Type>("8-neighbourhood weighted", k8Weighted),
	};
}

////////////////////

struct Neighbours_4 : public Neighbours {
	Neighbours_4(const V2i& size) : Neighbours(size) {
	}

	virtual void eval(const V2i& pos, const std::function<void(const V2i&, float)>& fn) const override {
		if(pos.x > 0)
			fn(V2i(pos.x-1, pos.y), 1);
		if(pos.x < size().x-1)
			fn(V2i(pos.x+1, pos.y), 1);
		if(pos.y > 0)
			fn(V2i(pos.x, pos.y-1), 1);
		if(pos.y < size().y-1)
			fn(V2i(pos.x, pos.y+1), 1);
	}
};

struct Neighbours_8 : public Neighbours {
	Neighbours_8(const V2i& size) : Neighbours(size) {
	}

	virtual void eval(const V2i& pos, const std::function<void(const V2i&, float)>& fn) const override {
		int min_x = std::max(0, pos.x-1);
		int min_y = std::max(0, pos.y-1);
		int max_x = std::min(pos.x+1, size().x-1);
		int max_y = std::min(pos.y+1, size().y-1);

		for(int y = min_y; y <= max_y; ++y)
			for(int x = min_x; x <= max_x; ++x)
				if(x != pos.x || y != pos.y)
					fn(V2i(x, y), 1);
	}
};

struct Neighbours_8_Weighted : public Neighbours {
	Neighbours_8_Weighted(const V2i& size) : Neighbours(size) {
	}

	virtual void eval(const V2i& pos, const std::function<void(const V2i&, float)>& fn) const {
		int min_x = std::max(0, pos.x-1);
		int min_y = std::max(0, pos.y-1);
		int max_x = std::min(pos.x+1, size().x-1);
		int max_y = std::min(pos.y+1, size().y-1);

		for(int y = min_y; y <= max_y; ++y)
			for(int x = min_x; x <= max_x; ++x)
				if(x != pos.x || y != pos.y)
					fn(V2i(x, y), 1 + (x == pos.x || y == pos.y));
	}
};

///////////////

std::unique_ptr<Neighbours> Neighbours::create(Type t, const V2i& size) {
	if(t == k4)
		return std::unique_ptr<Neighbours>(new Neighbours_4(size));
	else if(t == k8)
		return std::unique_ptr<Neighbours>(new Neighbours_8(size));
	else if(t == k8Weighted)
		return std::unique_ptr<Neighbours>(new Neighbours_8_Weighted(size));
	else {
		assert(false && "Unknown neighbourhood type");
		throw std::runtime_error("Unknown neighbourhood type specified.");
	}
}


}
