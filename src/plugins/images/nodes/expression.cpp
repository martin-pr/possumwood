#include <memory>

#include <exprtk/exprtk.hpp>

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/source_editor.h>

#include <QPainter>

#include "datatypes/pixmap.h"

namespace {

dependency_graph::InAttr<std::string> a_src;
dependency_graph::InAttr<std::shared_ptr<const QPixmap>> a_inPixmap;
dependency_graph::OutAttr<std::shared_ptr<const QPixmap>> a_outPixmap;

class Editor : public possumwood::SourceEditor {
	public:
		Editor() : SourceEditor(a_src) {
		}

};

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	std::shared_ptr<const QPixmap> input = data.get(a_inPixmap);

	if(input == nullptr) {
		result.addError("No input pixmap");
		data.set(a_outPixmap, input);
	}

	else {
		float x = 0.0f;
		float y = 0.0f;

		float r = 0.0f;
		float g = 0.0f;
		float b = 0.0f;

		exprtk::symbol_table<float> symbol_table;

		symbol_table.add_variable("x", x);
		symbol_table.add_variable("y", y);

		symbol_table.add_variable("r", r);
		symbol_table.add_variable("g", g);
		symbol_table.add_variable("b", b);

		symbol_table.add_constant("width", input->width());
		symbol_table.add_constant("height", input->height());

		symbol_table.add_constants();

		exprtk::expression<float> expression;
		expression.register_symbol_table(symbol_table);

		exprtk::parser<float> parser;

		const bool success = parser.compile(data.get(a_src), expression);

		auto cexpr = (const exprtk::expression<float>)expression;

		// compilation failed
		if(!success) {
			result.addError(parser.error());

			// output nothing
			data.set(a_outPixmap, std::shared_ptr<const QPixmap>());
		}

		// compilation success
		else {
			// make a QImage, so we can set pixels
			QImage image = input->toImage();

			// iterate over pixels
			for(int yi = 0; yi < input->height(); ++yi)
				for(int xi = 0; xi < input->width(); ++xi) {
					x = xi;
					y = yi;

					// r, g, b variables are changeable from the expression
					QColor color = image.pixelColor(xi, yi);
					r = color.redF();
					g = color.greenF();
					b = color.blueF();

					// evaluate the expression, ignoring its output
					cexpr.value();

					color.setRedF(r);
					color.setGreenF(g);
					color.setBlueF(b);
					image.setPixelColor(xi, yi, color);
				}

			std::unique_ptr<QPixmap> out(new QPixmap());
			*out = QPixmap::fromImage(std::move(image));

			data.set(a_outPixmap, std::shared_ptr<const QPixmap>(out.release()));
		}
	}

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_src, "source", std::string("r := x / width;\ng := y / height;\nb := max(0, 1-r-g);"));
	meta.addAttribute(a_inPixmap, "in_image");
	meta.addAttribute(a_outPixmap, "out_image");

	meta.addInfluence(a_src, a_outPixmap);
	meta.addInfluence(a_inPixmap, a_outPixmap);

	meta.setCompute(&compute);
	meta.setEditor<Editor>();
}

possumwood::NodeImplementation s_impl("images/expression", init);

}
