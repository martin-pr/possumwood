#include "grid.h"

namespace lightfields {

Grid::Grid(const V2i& size, int n_link_value, std::size_t layer_count) : m_size(size) {
	m_tLinks = std::vector<TLinks>(layer_count, TLinks(size));
	m_nLinks = std::vector<NLinks>(layer_count, NLinks(size, n_link_value));
}

std::size_t Grid::v2i(const V2i& v) const {
	return v.x + v.y*m_size.x;
}

void Grid::setValue(const V2i& pos, const std::vector<int>& values) {
	assert(values.size() == m_tLinks.size());

	auto tit = m_tLinks.begin();
	auto vit = values.begin();

	while(tit != m_tLinks.end()) {
		// TODO: generalize
		tit->edge(pos).setCapacity(*vit, std::numeric_limits<int>::max()/2);

		assert(*vit >= 0);
		assert(tit->edge(pos).forward().residualCapacity() == *vit);
		assert(tit->edge(pos).backward().residualCapacity() == std::numeric_limits<int>::max()/2);

		++tit;
		++vit;
	}
}

std::size_t Grid::layerCount() const {
	return m_tLinks.size();
}

V2i Grid::size() const {
	return m_size;
}

Link::Direction& Grid::edge(const Index& source, const Index& target) {
	assert(Index::sqdist(source, target) == 1);

	assert(source.n_layer <= layerCount());
	assert(target.n_layer <= layerCount());

	assert(source.pos.x >= 0 && source.pos.x < m_size.x);
	assert(source.pos.y >= 0 && source.pos.y < m_size.y);
	assert(target.pos.x >= 0 && target.pos.x < m_size.x);
	assert(target.pos.y >= 0 && target.pos.y < m_size.y);

#ifndef NDEBUG
	if(source.n_layer == layerCount())
		assert(target.n_layer < layerCount());
	else if(target.n_layer == layerCount())
		assert(source.n_layer < layerCount());
	else
		assert(source.n_layer < layerCount() && target.n_layer < layerCount());
#endif

	// same layer - need to use N link
	if(source.n_layer == target.n_layer)
		return m_nLinks[source.n_layer].edge(source.pos, target.pos);

	// different layer down - need to use a T link (if the move is to higher layer)
	else if(source.n_layer < target.n_layer) {
		assert(source.pos == target.pos);
		return m_tLinks[source.n_layer].edge(source.pos).forward();
	}

	// different layer up
	else {
		assert(source.n_layer > target.n_layer);
		assert(source.pos == target.pos);
		return m_tLinks[target.n_layer].edge(target.pos).backward();
	}
}

const Link::Direction& Grid::edge(const Index& source, const Index& target) const {
	assert(Index::sqdist(source, target) == 1);

	assert(source.n_layer <= layerCount());
	assert(target.n_layer <= layerCount());

	assert(source.pos.x >= 0 && source.pos.x < m_size.x);
	assert(source.pos.y >= 0 && source.pos.y < m_size.y);
	assert(target.pos.x >= 0 && target.pos.x < m_size.x);
	assert(target.pos.y >= 0 && target.pos.y < m_size.y);

#ifndef NDEBUG
	if(source.n_layer == layerCount())
		assert(target.n_layer < layerCount());
	else if(target.n_layer == layerCount())
		assert(source.n_layer < layerCount());
	else
		assert(source.n_layer < layerCount() && target.n_layer < layerCount());
#endif

	if(source.n_layer == target.n_layer)
		return m_nLinks[source.n_layer].edge(source.pos, target.pos);

	else if(source.n_layer < target.n_layer) {
		assert(source.pos == target.pos);
		return m_tLinks[source.n_layer].edge(source.pos).forward();
	}
	else {
		assert(source.n_layer > target.n_layer);
		assert(source.pos == target.pos);
		return m_tLinks[target.n_layer].edge(target.pos).backward();
	}
}

cv::Mat Grid::minCut() const {
	// collects all reachable nodes from source or sink, and marks them appropriately
	cv::Mat result = cv::Mat::zeros(m_size.y, m_size.x, CV_8UC1);

	// reachable from source as a trivial DFS search
	std::vector<bool> visited(m_size.x * m_size.y * (m_nLinks.size()+1));

	std::vector<Index> stack;
	for(int y=0; y<m_size.y; ++y)
		for(int x=0; x<m_size.x; ++x) {
			stack.push_back(Index{V2i(x, y), 0});

			while(!stack.empty()) {
				const Index current = stack.back();
				stack.pop_back();

				unsigned char& value = result.at<unsigned char>(current.pos.y, current.pos.x);
				const int target = (current.n_layer) * (255 / (m_nLinks.size() - 1));

				if(!visited[v2i(current.pos) + current.n_layer * m_size.x * m_size.y]) {
					visited[v2i(current.pos) + current.n_layer * m_size.x * m_size.y] = true;

					if(target > value)
						value = target;

					if(current.n_layer < m_nLinks.size()) {
						// horizontal move
						if(current.pos.x > 0 && m_nLinks[current.n_layer].edge(current.pos, V2i(current.pos.x-1, current.pos.y)).residualCapacity() > 0)
							stack.push_back(Index{V2i(current.pos.x-1, current.pos.y), current.n_layer});
						if(current.pos.x < m_size.x-1 && m_nLinks[current.n_layer].edge(current.pos, V2i(current.pos.x+1, current.pos.y)).residualCapacity() > 0)
							stack.push_back(Index{V2i(current.pos.x+1, current.pos.y), current.n_layer});

						// vertical move
						if(current.pos.y > 0 && m_nLinks[current.n_layer].edge(current.pos, V2i(current.pos.x, current.pos.y-1)).residualCapacity() > 0)
							stack.push_back(Index{V2i(current.pos.x, current.pos.y-1), current.n_layer});
						if(current.pos.y < m_size.y-1 && m_nLinks[current.n_layer].edge(current.pos, V2i(current.pos.x, current.pos.y+1)).residualCapacity() > 0)
							stack.push_back(Index{V2i(current.pos.x, current.pos.y+1), current.n_layer});

						// layer move down
						assert(current.n_layer < m_nLinks.size());
						if(m_tLinks[current.n_layer].edge(current.pos).forward().residualCapacity() > 0)
							stack.push_back(Index{current.pos, current.n_layer+1});

						// layer move up (unconditional)
						if(current.n_layer > 0 && m_tLinks[current.n_layer].edge(current.pos).backward().residualCapacity() > 0)
							stack.push_back(Index{current.pos, current.n_layer-1});
					}
				}
			}
		}

	return result;
}

std::vector<cv::Mat> Grid::debug() const {
	std::vector<cv::Mat> result;
	for(std::size_t a=0;a<m_nLinks.size(); ++a) {
		result.push_back(n_flow(m_nLinks[a]));
		result.push_back(t_flow(m_tLinks[a]));
	}

	return result;
}

cv::Mat Grid::t_flow(const TLinks& t) const {
	cv::Mat result(m_size.y, m_size.x, CV_8UC1);

	for(int y=0; y<m_size.y; ++y)
		for(int x=0; x<m_size.x; ++x) {
			auto& e = t.edge(V2i(x, y));
			if(e.forward().capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.forward().flow()) * 255 / e.forward().capacity();
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}

cv::Mat Grid::n_flow(const NLinks& n) const {
	cv::Mat result(m_size.y, m_size.x, CV_8UC1);

	for(int y=0; y<m_size.y; ++y)
		for(int x=0; x<m_size.x; ++x) {
			float residual = 0.0f;
			float max = 0.0f;
			if(x > 0) {
				auto& e = n.edge(V2i(x-1, y), V2i(x, y));
				if(e.capacity() > 0) {
					residual += (float)e.residualCapacity();
					max += (float)e.capacity();
				}
			}

			if(x < m_size.x-1) {
				auto& e = n.edge(V2i(x+1, y), V2i(x, y));
				if(e.capacity() > 0) {
					residual += (float)e.residualCapacity();
					max += (float)e.capacity();
				}
			}

			if(y > 0) {
				auto& e = n.edge(V2i(x, y-1), V2i(x, y));
				if(e.capacity() > 0) {
					residual += (float)e.residualCapacity();
					max += (float)e.capacity();
				}
			}

			if(y < m_size.y-1) {
				auto& e = n.edge(V2i(x, y+1), V2i(x, y));
				if(e.capacity() > 0) {
					residual += (float)e.residualCapacity();
					max += (float)e.capacity();
				}
			}

			if(max > 0.0f)
				result.at<unsigned char>(y,x) = 255.0f - residual / max * 255.0f;
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}
}
