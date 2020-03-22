#pragma once

#include <vector>
#include <set>

#include <opencv2/opencv.hpp>

#include <tbb/blocked_range2d.h>

#include "grid.h"

#include "active_queue.h"

namespace lightfields {

class BFSVisitors;
class Labels;

class Graph {
	public:
		void edmondsKarpSolve(Grid& grid);
		void pushRelabelSolve(Grid& grid);

	private:
		bool iterate(Grid& grid, const tbb::blocked_range2d<int>& range);

		int flow(Grid& grid, const BFSVisitors& visitors, const Index& end) const;
		void doFlow(Grid& grid, const BFSVisitors& visitors, const Index& end, int value);

		bool pushHoriz(Grid& grid, const Index& current, const Index& next, const Labels& label, int& excess, ActiveQueue& queue);
		bool pushVertDown(Grid& grid, const Index& current, const Labels& label, int& excess, ActiveQueue& queue);
		bool pushVertUp(Grid& grid, const Index& current, const Labels& label, int& excess, ActiveQueue& queue);
		bool relabel(const Grid& grid, const Index& current, Labels& label) const;

		std::size_t relabelAll(const Grid& grid, Labels& labels, ActiveQueue& queue);
};

}
