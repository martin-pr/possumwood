#include <possumwood_sdk/node_implementation.h>

#include "datatypes/symbol_table.h"

namespace {

dependency_graph::InAttr<std::string> a_name;
dependency_graph::InAttr<float> a_value;
dependency_graph::InAttr<possumwood::ExprSymbols> a_inSymbols;
dependency_graph::OutAttr<possumwood::ExprSymbols> a_outSymbols;

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::ExprSymbols table = data.get(a_inSymbols);
	table.addConstant(data.get(a_name), data.get(a_value));
	data.set(a_outSymbols, table);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_name, "name", std::string("constant"));
	meta.addAttribute(a_value, "value", 0.0f);
	meta.addAttribute(a_inSymbols, "in_symbols");

	meta.addAttribute(a_outSymbols, "out_symbols");

	meta.addInfluence(a_name, a_outSymbols);
	meta.addInfluence(a_value, a_outSymbols);
	meta.addInfluence(a_inSymbols, a_outSymbols);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("expressions/float_constant", init);

}
