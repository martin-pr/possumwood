#include "values.h"

namespace dependency_graph {

Values::Values(Node& n) : m_node(&n) {
}

Values::Values(Values&& vals) : m_node(vals.m_node) {
}

}
