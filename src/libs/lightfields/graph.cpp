#include "graph.h"

#include <cassert>
#include <map>
#include <queue>

namespace lightfields {

Graph::Path::Path() : source(-1, -1), sink(-1, -1) {
}

namespace {

int sqdist(const Imath::V2i& v1, const Imath::V2i& v2) {
	return (v2[0]-v1[0]) * (v2[0]-v1[0]) + (v2[1]-v1[1]) * (v2[1]-v1[1]);
}

}

bool Graph::Path::isValid() const {
	bool result = source[0] >= 0 && source[1] >= 0 && sink[0] >= 0 && sink[1] >= 0;

	if(n_links.empty())
		result &= (sqdist(source, sink) == 0);

	else {
		result &= (sqdist(source, n_links.front()) == 0);

		if(!n_links.empty()) {
			auto it1 = n_links.begin();
			auto it2 = it1+1;
			while(it2 != n_links.end()) {
				result &= (sqdist(*it1, *it2) == 1);

				++it1;
				++it2;
			}
		}

		result &= (sqdist(sink, n_links.back()) == 0);
	}

	return result;
}


/////

Graph::Graph(const Imath::V2i& size, float n_link_value) : m_size(size), m_sourceLinks(size), m_sinkLinks(size), m_nLinks(size, n_link_value) {
}

void Graph::setValue(const Imath::V2i& pos, float source_weight, float sink_weight) {
	m_sourceLinks.edge(pos).setCapacity(source_weight);
	m_sinkLinks.edge(pos).setCapacity(sink_weight);
}

bool Graph::bfs(Path& path) const {
	path.n_links.clear();

	std::map<Imath::V2i, Imath::V2i, SetComparator> visited;
	std::queue<Imath::V2i> q;

	// set the "origin" visited nodes
	for(int siy=0; siy<m_size[1]; ++siy) {
		for(int six=0; six<m_size[0]; ++six) {
			if(m_sourceLinks.edge(Imath::V2i(six, siy)).residualCapacity() > 0.0f) {
				visited[Imath::V2i(six, siy)] = Imath::V2i(-1, -1);
				q.push(Imath::V2i(six, siy));
			}
		}
	}

	// the core of the algorithm
	while(!q.empty()) {
		Imath::V2i current = q.front();
		q.pop();

		// check if there is an exit point here
		if(m_sinkLinks.edge(current).residualCapacity() > 0.0f) {
			path.n_links.clear();

			path.sink = current;

			// this is SLOW, but for now will do
			while(current[0] >= 0) {
				path.n_links.insert(path.n_links.begin(), current);
				current = visited[current];
			}

			path.source = path.n_links.front();

			assert(path.isValid());

			return true;
		}

		// try to move horizontally left
		if(current[0] > 0) {
			const Imath::V2i new_index(current[0]-1, current[1]);
			if(visited.find(new_index) == visited.end()) {
				visited[new_index] = current;

				if(m_nLinks.edge(current, new_index).residualCapacity() > 0.0f)
					q.push(new_index);
			}
		}

		// try to move horizontally left
		if(current[0] < m_size[0]-1) {
			const Imath::V2i new_index(current[0]+1, current[1]);
			if(visited.find(new_index) == visited.end()) {
				visited[new_index] = current;

				if(m_nLinks.edge(current, new_index).residualCapacity() > 0.0f)
					q.push(new_index);
			}
		}

		// try to move vertically up
		if(current[1] > 0) {
			const Imath::V2i new_index(current[0], current[1]-1);
			if(visited.find(new_index) == visited.end()) {
				visited[new_index] = current;

				if(m_nLinks.edge(current, new_index).residualCapacity() > 0.0f)
					q.push(new_index);
			}
		}

		// try to move vertically down
		if(current[1] < m_size[1]-1) {
			const Imath::V2i new_index(current[0], current[1]+1);
			if(visited.find(new_index) == visited.end()) {
				visited[new_index] = current;

				if(m_nLinks.edge(current, new_index).residualCapacity() > 0.0f)
					q.push(new_index);
			}
		}
	}

	return false;
}

float Graph::flow(const Path& path) const {
	assert(path.isValid());

	float result = m_sourceLinks.edge(path.source).residualCapacity();

	if(!path.n_links.empty()) {
		auto it1 = path.n_links.begin();
		auto it2 = it1+1;

		while(it2 != path.n_links.end()) {
			result = std::min(result, m_nLinks.edge(*it1, *it2).residualCapacity());

			++it1;
			++it2;
		}
	}

	result = std::min(result, m_sinkLinks.edge(path.sink).residualCapacity());

	return result;
}

// void Graph::collect(std::set<Imath::V2i, Graph::SetComparator>& subgraph, const Imath::V2i& i) const {
// 	if(subgraph.find(i) == subgraph.end()) {
// 		subgraph.insert(i);

// 		if(i[0] > 0) {
// 			auto& e = m_nLinks.edge(i, Imath::V2i(i[0]-1, i[1]));
// 			if(std::abs(e.flow()) < e.capacity())
// 				collect(subgraph, Imath::V2i(i[0]-1, i[1]));
// 			else
// 				assert(std::abs(e.flow()) - e.capacity() < Graph2D::flowEPS());
// 		}

// 		if(i[0] < m_size[0]-1) {
// 			auto& e = m_nLinks.edge(i, Imath::V2i(i[0]+1, i[1]));
// 			if(std::abs(e.flow()) < e.capacity())
// 				collect(subgraph, Imath::V2i(i[0]+1, i[1]));
// 			else
// 				assert(std::abs(e.flow()) - e.capacity() < Graph2D::flowEPS());
// 		}

// 		if(i[1] > 0) {
// 			auto& e = m_nLinks.edge(i, Imath::V2i(i[0], i[1]-1));
// 			if(std::abs(e.flow()) < e.capacity())
// 				collect(subgraph, Imath::V2i(i[0], i[1]-1));
// 			else
// 				assert(std::abs(e.flow()) - e.capacity() < Graph2D::flowEPS());
// 		}

// 		if(i[1] < m_size[1]-1) {
// 			auto& e = m_nLinks.edge(i, Imath::V2i(i[0], i[1]+1));
// 			if(std::abs(e.flow()) < e.capacity())
// 				collect(subgraph, Imath::V2i(i[0], i[1]+1));
// 			else
// 				assert(std::abs(e.flow()) - e.capacity() < Graph2D::flowEPS());
// 		}
// 	}
// }

void Graph::solve() {
	Path path;
	while(bfs(path)) {
		assert(path.isValid());

		// get the maximum flow through the path
		const float f = flow(path);
		assert(f > 0.0f);

		// update the graph
		{
		// 	std::cout << "before: " << f << "  ->  ";
		// 	{
		// 		std::cout << "(" << m_sourceLinks.edge(path.source).residualCapacity() << ") ";

		// 		auto it1 = path.n_links.begin();
		// 		auto it2 = it1+1;
		// 		while(it2 != path.n_links.end()) {
		// 			std::cout << (*it1)[0] << "," << (*it1)[1] << " (" << m_nLinks.edge(*it1, *it2).residualCapacity() << ") ";

		// 			++it1;
		// 			++it2;
		// 		}

		// 		std::cout << (*it1)[0] << "," << (*it1)[1] << " ";

		// 		std::cout << "(" << m_sinkLinks.edge(path.sink).residualCapacity() << ") " << std::endl;
		// 	}

			{
				auto& e = m_sourceLinks.edge(path.source);
				e.addFlow(f);
			}

			if(!path.n_links.empty()) {
				auto it1 = path.n_links.begin();
				auto it2 = it1+1;
				while(it2 != path.n_links.end()) {
					m_nLinks.edge(*it1, *it2).addFlow(f);

					assert(m_nLinks.edge(*it1, *it2).residualCapacity() >= -Graph2D::flowEPS());
					assert(m_nLinks.edge(*it2, *it1).residualCapacity() >= -Graph2D::flowEPS());

					++it1;
					++it2;
				}
			}

			{
				auto& e = m_sinkLinks.edge(path.sink);
				e.addFlow(f);
			}
		}

		// std::cout << "after: " << f << "  ->  ";
		// {
		// 	std::cout << "(" << m_sourceLinks.edge(path.source).residualCapacity() << ") ";

		// 	auto it1 = path.n_links.begin();
		// 	auto it2 = it1+1;
		// 	while(it2 != path.n_links.end()) {
		// 		std::cout << (*it1)[0] << "," << (*it1)[1] << " (" << m_nLinks.edge(*it1, *it2).residualCapacity() << ") ";

		// 		++it1;
		// 		++it2;
		// 	}

		// 	std::cout << (*it1)[0] << "," << (*it1)[1] << " ";

		// 	std::cout << "(" << m_sinkLinks.edge(path.sink).residualCapacity() << ") " << std::endl;
		// }

		assert(flow(path) < Graph2D::flowEPS());
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
			result.at<unsigned char>(y, x) = /* 127 */ 0;

	// reachable from source
	{
		std::vector<Imath::V2i> stack;
		for(int y=0; y<m_size[1]; ++y)
			for(int x=0; x<m_size[0]; ++x)
				if(m_sourceLinks.edge(Imath::V2i(x, y)).residualCapacity() > 0.0f) {
					stack.push_back(Imath::V2i(x, y));

					while(!stack.empty()) {
						const Imath::V2i current = stack.back();
						stack.pop_back();

						unsigned char& value = result.at<unsigned char>(current[1], current[0]);
						if(value < 255) {
							result.at<unsigned char>(current[1], current[0]) = 255;

							if(current[0] > 0 && m_nLinks.edge(current, Imath::V2i(current[0]-1, current[1])).residualCapacity() > 0.0f)
								stack.push_back(Imath::V2i(current[0]-1, current[1]));
							if(current[0] < m_size[0]-1 && m_nLinks.edge(current, Imath::V2i(current[0]+1, current[1])).residualCapacity() > 0.0f)
								stack.push_back(Imath::V2i(current[0]+1, current[1]));

							if(current[1] > 0 && m_nLinks.edge(current, Imath::V2i(current[0], current[1]-1)).residualCapacity() > 0.0f)
								stack.push_back(Imath::V2i(current[0], current[1]-1));
							if(current[1] < m_size[1]-1 && m_nLinks.edge(current, Imath::V2i(current[0], current[1]+1)).residualCapacity() > 0.0f)
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
			if(e.capacity() > 0.0f)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) / e.capacity() * 255.0f;
			else
				result.at<unsigned char>(y,x) = 0;
		}

	return result;
}

cv::Mat Graph::sinkFlow() const {
	cv::Mat result(m_size[1], m_size[0], CV_8UC1);

	for(int y=0; y<m_size[1]; ++y)
		for(int x=0; x<m_size[0]; ++x) {
			auto& e = m_sinkLinks.edge(Imath::V2i(x, y));
			if(e.capacity() > 0.0f)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) / e.capacity() * 255.0f;
			else
				result.at<unsigned char>(y,x) = 0;
		}

	return result;
}

cv::Mat Graph::horizontalFlow() const {
	cv::Mat result = cv::Mat::zeros(m_size[1], m_size[0], CV_8UC1);

	for(int y=0; y<m_size[1]; ++y)
		for(int x=0; x<m_size[0]-1; ++x) {
			auto& e = m_nLinks.edge(Imath::V2i(x, y), Imath::V2i(x+1, y));
			if(e.capacity() > 0.0f)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) / e.capacity() * 255.0f;
			else
				result.at<unsigned char>(y,x) = 0;
		}

	return result;
}

cv::Mat Graph::verticalFlow() const {
	cv::Mat result = cv::Mat::zeros(m_size[1], m_size[0], CV_8UC1);

	for(int y=0; y<m_size[1]-1; ++y)
		for(int x=0; x<m_size[0]; ++x) {
			auto& e = m_nLinks.edge(Imath::V2i(x, y), Imath::V2i(x, y+1));
			if(e.capacity() > 0.0f)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) / e.capacity() * 255.0f;
			else
				result.at<unsigned char>(y,x) = 0;
		}

	return result;
}



// std::set<Imath::V2i, Graph::SetComparator> Graph::sinkGraph() const {
// 	std::set<Imath::V2i, Graph::SetComparator> result;
// 	for(int y=0; y<m_size[1]; ++y)
// 		for(int x=0; x<m_size[0]; ++x)
// 			if(m_sinkLinks.edge(Imath::V2i(x, y)).residualCapacity() > 0.0f)
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
