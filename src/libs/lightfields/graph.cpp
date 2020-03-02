#include "graph.h"

#include <cassert>
#include <map>
#include <queue>

namespace lightfields {

Graph::Path::Path() {
}

namespace {

int sqdist(const Imath::V2i& v1, const Imath::V2i& v2) {
	return (v2[0]-v1[0]) * (v2[0]-v1[0]) + (v2[1]-v1[1]) * (v2[1]-v1[1]);
}

}

bool Graph::Path::isValid() const {
	if(n_links.empty())
		return false;

	else {
		bool result = true;

		auto it1 = n_links.begin();
		auto it2 = it1+1;
		while(it2 != n_links.end()) {
			result &= (sqdist(*it1, *it2) == 1);

			++it1;
			++it2;
		}
		return result;
	}
}


/////

Graph::Graph(const Imath::V2i& size, int n_link_value) : m_size(size), m_sourceLinks(size), m_sinkLinks(size), m_nLinks(size, n_link_value) {
}

void Graph::setValue(const Imath::V2i& pos, int source_weight, int sink_weight) {
	assert(source_weight >= 0.0f);
	assert(sink_weight >= 0.0f);

	// if(source_weight > sink_weight) {
	// 	source_weight -= sink_weight;
	// 	sink_weight = 0.0f;
	// }
	// else {
	// 	sink_weight -= source_weight;
	// 	source_weight = 0.0f;
	// }

	m_sourceLinks.edge(pos).setCapacity(source_weight);
	m_sinkLinks.edge(pos).setCapacity(sink_weight);
}

bool Graph::bfs_2(Path& path, std::size_t& offset) const {
	path.n_links.clear();

	std::map<Imath::V2i, Imath::V2i, SetComparator> visited;
	std::queue<Imath::V2i> q;

	const std::size_t end = m_size[0] * m_size[1];
	for(std::size_t i=0; i<end; ++i) {
		const std::size_t id = (i + offset) % end;
		const Imath::V2i src(id % m_size[0], id / m_size[0]);

		if(m_sourceLinks.edge(src).residualCapacity() > 0) {
			visited.clear();
			visited[src] = Imath::V2i(-1, -1);

			assert(q.empty());
			q.push(src);

			// the core of the algorithm
			while(!q.empty()) {
				Imath::V2i current = q.front();
				q.pop();

				// check if there is an exit point here
				if(m_sinkLinks.edge(current).residualCapacity() > 0) {
					path.n_links.clear();

					// this is SLOW, but for now will do
					while(current[0] >= 0) {
						path.n_links.push_back(current);
						current = visited[current];
					}
					std::reverse(path.n_links.begin(), path.n_links.end());

					assert(path.isValid());

					offset = id+1;

					return true;
				}

				// try to move horizontally left
				if(current[0] > 0) {
					const Imath::V2i new_index(current[0]-1, current[1]);
					if(visited.find(new_index) == visited.end()) {
						if(m_nLinks.edge(current, new_index).residualCapacity() > 0) {
							visited[new_index] = current;
							q.push(new_index);
						}
					}
				}

				// try to move horizontally right
				if(current[0] < m_size[0]-1) {
					const Imath::V2i new_index(current[0]+1, current[1]);
					if(visited.find(new_index) == visited.end()) {
						if(m_nLinks.edge(current, new_index).residualCapacity() > 0) {
							visited[new_index] = current;
							q.push(new_index);
						}
					}
				}

				// try to move vertically up
				if(current[1] > 0) {
					const Imath::V2i new_index(current[0], current[1]-1);
					if(visited.find(new_index) == visited.end()) {
						if(m_nLinks.edge(current, new_index).residualCapacity() > 0) {
							visited[new_index] = current;
							q.push(new_index);
						}
					}
				}

				// try to move vertically down
				if(current[1] < m_size[1]-1) {
					const Imath::V2i new_index(current[0], current[1]+1);
					if(visited.find(new_index) == visited.end()) {
						if(m_nLinks.edge(current, new_index).residualCapacity() > 0) {
							visited[new_index] = current;
							q.push(new_index);
						}
					}
				}
			}
		}
	}



	return false;
}

int Graph::flow(const Path& path) const {
	assert(path.isValid());

	int result = m_sourceLinks.edge(path.n_links.front()).residualCapacity();

	if(!path.n_links.empty()) {
		auto it1 = path.n_links.begin();
		auto it2 = it1+1;

		while(it2 != path.n_links.end()) {
			result = std::min(result, m_nLinks.edge(*it1, *it2).residualCapacity());

			++it1;
			++it2;
		}
	}

	result = std::min(result, m_sinkLinks.edge(path.n_links.back()).residualCapacity());

	return result;
}

void Graph::solve() {
	Path path;

	// while(bfs_1(path)) {
	// 	assert(path.n_links.empty());
	// 	assert(path.source == path.sink);

	// 	const int f = flow(path);
	// 	assert(f > 0.0f);

	// 	m_sourceLinks.edge(path.source).addFlow(f);
	// 	m_sinkLinks.edge(path.sink).addFlow(f);

	// 	assert(flow(path) < flowEPS());
	// }

	std::size_t offset = 0;
	while(bfs_2(path, offset)) {
		assert(path.isValid());


		// get the maximum flow through the path
		const int f = flow(path);
		assert(f > 0);

		// update the graph
		{
			auto& e = m_sourceLinks.edge(path.n_links.front());
			e.addFlow(f);
		}

		if(!path.n_links.empty()) {
			auto it1 = path.n_links.begin();
			auto it2 = it1+1;
			while(it2 != path.n_links.end()) {
				m_nLinks.edge(*it1, *it2).addFlow(f);

				assert(m_nLinks.edge(*it1, *it2).residualCapacity() >= 0);
				assert(m_nLinks.edge(*it2, *it1).residualCapacity() >= 0);

				++it1;
				++it2;
			}
		}

		{
			auto& e = m_sinkLinks.edge(path.n_links.back());
			e.addFlow(f);
		}

		assert(flow(path) == 0);
	}
}

// std::set<Imath::V2i, Graph::SetComparator> Graph::sourceGraph() const {
// 	std::set<Imath::V2i, Graph::SetComparator> result;
// 	for(int y=0; y<m_size[1]; ++y)
// 		for(int x=0; x<m_size[0]; ++x) {
// 			auto& e = m_sourceLinks.edge(Imath::V2i(x, y));
// 			if(std::abs(e.flow()) < e.capacity())
// 				collect(result, Imath::V2i(x, y));
// 		}

// 	// std::cout << "source graph:" << std::endl;
// 	// std::cout << "  size: " << result.size() << std::endl;
// 	// std::cout << "  total: " << (m_size[0]*m_size[1]) << std::endl;
// 	// std::cout << "  ";
// 	// for(auto& p : result)
// 	// 	std::cout << " " << p[0] << "," << p[1];
// 	// std::cout << std::endl;

// 	return result;
// }

cv::Mat Graph::minCut() const {
	// collects all reachable nodes from source or sink, and marks them appropriately
	cv::Mat result(m_size[1], m_size[0], CV_8UC1);
	for(int y=0; y<m_size[1]; ++y)
		for(int x=0; x<m_size[0]; ++x)
			result.at<unsigned char>(y, x) = 127;

	// reachable from source
	{
		std::vector<Imath::V2i> stack;
		for(int y=0; y<m_size[1]; ++y)
			for(int x=0; x<m_size[0]; ++x)
				if(m_sourceLinks.edge(Imath::V2i(x, y)).residualCapacity() > 0) {
					stack.push_back(Imath::V2i(x, y));

					while(!stack.empty()) {
						const Imath::V2i current = stack.back();
						stack.pop_back();

						assert(m_sinkLinks.edge(current).residualCapacity() == 0 && "TODO: FIX THIS TRIGGER!!!");

						unsigned char& value = result.at<unsigned char>(current[1], current[0]);
						if(value < 255) {
							result.at<unsigned char>(current[1], current[0]) = 255;

							if(current[0] > 0 && m_nLinks.edge(current, Imath::V2i(current[0]-1, current[1])).residualCapacity() > 0)
								stack.push_back(Imath::V2i(current[0]-1, current[1]));
							if(current[0] < m_size[0]-1 && m_nLinks.edge(current, Imath::V2i(current[0]+1, current[1])).residualCapacity() > 0)
								stack.push_back(Imath::V2i(current[0]+1, current[1]));

							if(current[1] > 0 && m_nLinks.edge(current, Imath::V2i(current[0], current[1]-1)).residualCapacity() > 0)
								stack.push_back(Imath::V2i(current[0], current[1]-1));
							if(current[1] < m_size[1]-1 && m_nLinks.edge(current, Imath::V2i(current[0], current[1]+1)).residualCapacity() > 0)
								stack.push_back(Imath::V2i(current[0], current[1]+1));
						}
					}
				}
	}

	// reachable from sink
	{
		std::vector<Imath::V2i> stack;
		for(int y=0; y<m_size[1]; ++y)
			for(int x=0; x<m_size[0]; ++x)
				if(m_sinkLinks.edge(Imath::V2i(x, y)).residualCapacity() > 0) {
					stack.push_back(Imath::V2i(x, y));

					while(!stack.empty()) {
						const Imath::V2i current = stack.back();
						stack.pop_back();

						assert(m_sourceLinks.edge(Imath::V2i(x, y)).residualCapacity() == 0);

						unsigned char& value = result.at<unsigned char>(current[1], current[0]);
						if(value > 0) {
							result.at<unsigned char>(current[1], current[0]) = 0;

							if(current[0] > 0 && m_nLinks.edge(Imath::V2i(current[0]-1, current[1]), current).residualCapacity() > 0)
								stack.push_back(Imath::V2i(current[0]-1, current[1]));
							if(current[0] < m_size[0]-1 && m_nLinks.edge(Imath::V2i(current[0]+1, current[1]), current).residualCapacity() > 0)
								stack.push_back(Imath::V2i(current[0]+1, current[1]));

							if(current[1] > 0 && m_nLinks.edge(Imath::V2i(current[0], current[1]-1), current).residualCapacity() > 0)
								stack.push_back(Imath::V2i(current[0], current[1]-1));
							if(current[1] < m_size[1]-1 && m_nLinks.edge(Imath::V2i(current[0], current[1]+1), current).residualCapacity() > 0)
								stack.push_back(Imath::V2i(current[0], current[1]+1));
						}
					}
				}
	}

	return result;
}

cv::Mat Graph::sourceFlow() const {
	cv::Mat result(m_size[1], m_size[0], CV_8UC1);

	for(int y=0; y<m_size[1]; ++y)
		for(int x=0; x<m_size[0]; ++x) {
			auto& e = m_sourceLinks.edge(Imath::V2i(x, y));
			if(e.capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) * 255 / e.capacity();
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}

cv::Mat Graph::sinkFlow() const {
	cv::Mat result(m_size[1], m_size[0], CV_8UC1);

	for(int y=0; y<m_size[1]; ++y)
		for(int x=0; x<m_size[0]; ++x) {
			auto& e = m_sinkLinks.edge(Imath::V2i(x, y));
			if(e.capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) * 255 / e.capacity();
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}

cv::Mat Graph::horizontalFlow() const {
	cv::Mat result = cv::Mat::zeros(m_size[1], m_size[0], CV_8UC1);

	for(int y=0; y<m_size[1]; ++y)
		for(int x=0; x<m_size[0]-1; ++x) {
			auto& e = m_nLinks.edge(Imath::V2i(x, y), Imath::V2i(x+1, y));
			if(e.capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) * 255 / e.capacity();
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}

cv::Mat Graph::verticalFlow() const {
	cv::Mat result = cv::Mat::zeros(m_size[1], m_size[0], CV_8UC1);

	for(int y=0; y<m_size[1]-1; ++y)
		for(int x=0; x<m_size[0]; ++x) {
			auto& e = m_nLinks.edge(Imath::V2i(x, y), Imath::V2i(x, y+1));
			if(e.capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) * 255 / e.capacity();
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}



// std::set<Imath::V2i, Graph::SetComparator> Graph::sinkGraph() const {
// 	std::set<Imath::V2i, Graph::SetComparator> result;
// 	for(int y=0; y<m_size[1]; ++y)
// 		for(int x=0; x<m_size[0]; ++x)
// 			if(m_sinkLinks.edge(Imath::V2i(x, y)).residualCapacity() > flowEPS())
// 				collect(result, Imath::V2i(x, y));

// 	std::cout << "sink graph:" << std::endl;
// 	std::cout << "  size: " << result.size() << std::endl;
// 	std::cout << "  total: " << (m_size[0]*m_size[1]) << std::endl;
// 	std::cout << "  ";
// 	for(auto& p : result)
// 		std::cout << " " << p[0] << "," << p[1];
// 	std::cout << std::endl;

// 	return result;
// }

}
