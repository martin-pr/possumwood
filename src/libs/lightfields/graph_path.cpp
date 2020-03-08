#include "graph_path.h"

#include <cassert>

namespace lightfields {

GraphPath::GraphPath() {
}

bool GraphPath::isValid() const {
	if(n_links.empty())
		return false;

	else {
		bool result = true;

		auto it1 = n_links.begin();
		auto it2 = it1+1;
		while(it2 != n_links.end()) {
			result &= (Index::sqdist(*it1, *it2) == 1);

			++it1;
			++it2;
		}
		return result;
	}
}

bool GraphPath::empty() const {
	return n_links.empty();
}

void GraphPath::add(const Index& link) {
	assert(n_links.empty() || (Index::sqdist(link, n_links.back()) == 1));
	n_links.push_back(link);
}

void GraphPath::clear() {
	n_links.clear();
}

GraphPath::const_iterator GraphPath::begin() const {
	return n_links.rbegin();
}

GraphPath::const_iterator GraphPath::end() const {
	return n_links.rend();
}

const Index& GraphPath::front() const {
	assert(!n_links.empty());
	return n_links.back();
}

const Index& GraphPath::back() const {
	assert(!n_links.empty());
	return n_links.front();
}

}
