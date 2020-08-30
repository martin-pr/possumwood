#pragma once

namespace lightfields {

class Graph;

struct PushRelabel {
	static void solve(Graph& grid);
};

}  // namespace lightfields
