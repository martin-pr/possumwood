#include <exprtk/exprtk.hpp>

#include <memory>

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/source_editor.h>

#include <QPainter>
#include <QPushButton>
#include <QMenu>

#include "datatypes/pixmap.h"
#include "expressions/datatypes/symbol_table.h"

namespace {

template<typename PIXMAP>
struct Params {
	dependency_graph::InAttr<std::string> a_src;
	dependency_graph::InAttr<possumwood::ExprSymbols> a_symbols;
	dependency_graph::InAttr<std::shared_ptr<const PIXMAP>> a_inPixmap;
	dependency_graph::OutAttr<std::shared_ptr<const PIXMAP>> a_outPixmap;
};

Params<possumwood::LDRPixmap> s_ldrParams;
Params<possumwood::HDRPixmap> s_hdrParams;

class Popup : public QMenu {
	public:
		Popup(QWidget* parent) : QMenu(parent) {
		}

		void addItem(const std::string& name) {
			addAction(name.c_str());
		}

	private:
};

template<typename PIXMAP>
class Editor : public possumwood::SourceEditor {
	public:
		Editor(Params<PIXMAP>& params) : SourceEditor(params.a_src), m_params(params), m_popup(nullptr) {
			m_varsButton = new QPushButton("Variables");
			buttonsLayout()->insertWidget(0, m_varsButton);

			widget()->connect(m_varsButton, &QPushButton::pressed, [this]() {
				if(m_popup)
					m_popup->popup(
						m_varsButton->mapToGlobal(QPoint(0,-m_popup->sizeHint().height()))
					);
			});
		}

	protected:
		virtual void valueChanged(const dependency_graph::Attr& attr) override {
			if(attr == m_params.a_symbols) {
				m_popup->deleteLater();

				populateVariableList();
			}

			else
				SourceEditor::valueChanged(attr);
		}

		void populateVariableList() {
			if(!m_popup)
				m_popup->deleteLater();

			m_popup = new Popup(m_varsButton);

			m_popup->connect(m_popup, &Popup::triggered, [this](QAction* action) {
				QString text = action->text();
				while(!text.isEmpty() && !QChar(text[0]).isLetterOrNumber())
					text = text.mid(1, text.length()-1);
				if(text.indexOf('\t') >= 0)
					text = text.mid(0, text.indexOf('\t'));

				editorWidget()->insertPlainText(text);
			});

			// the external symbols
			unsigned ctr = 0;
			for(auto& s : values().get(m_params.a_symbols)) {
				m_popup->addItem(s.first + "\t(constant)");
				++ctr;
			}

			if(ctr > 0)
				m_popup->addSeparator();

			// hardwired symbols
			m_popup->addItem("x\t(constant)");
			m_popup->addItem("y\t(constant)");
			m_popup->addItem("width\t(constant)");
			m_popup->addItem("height\t(constant)");

			m_popup->addSeparator();

			m_popup->addItem("r\t(variable)");
			m_popup->addItem("g\t(variable)");
			m_popup->addItem("b\t(variable)");
		}

	private:
		Params<PIXMAP>& m_params;

		QPushButton* m_varsButton;

		Popup* m_popup;
};

std::tuple<float, float, float> readValue(const possumwood::LDRPixel& pixel) {
	std::tuple<float, float, float> result;

	std::get<0>(result) = (float)pixel.value()[0] / 255.0f;
	std::get<1>(result) = (float)pixel.value()[1] / 255.0f;
	std::get<2>(result) = (float)pixel.value()[2] / 255.0f;

	return result;
};

std::tuple<float, float, float> readValue(const possumwood::HDRPixel& pixel) {
	return std::tie(pixel.value()[0], pixel.value()[1], pixel.value()[2]);
};

void writeValue(possumwood::LDRPixel& pixel, const std::tuple<float, float, float>& value) {
	pixel = possumwood::LDRPixmap::pixel_t::value_t{{
		possumwood::LDRPixmap::channel_t(std::min(255.0f, std::max(0.0f, std::get<0>(value) * 255.0f))),
		possumwood::LDRPixmap::channel_t(std::min(255.0f, std::max(0.0f, std::get<1>(value) * 255.0f))),
		possumwood::LDRPixmap::channel_t(std::min(255.0f, std::max(0.0f, std::get<2>(value) * 255.0f)))
	}};
};

void writeValue(possumwood::HDRPixel& pixel, const std::tuple<float, float, float>& value) {
	pixel = possumwood::HDRPixmap::pixel_t::value_t{{
		std::get<0>(value), std::get<1>(value), std::get<2>(value)
	}};
}

template<typename PIXMAP>
dependency_graph::State compute(dependency_graph::Values& data, Params<PIXMAP>& params) {
	dependency_graph::State result;

	std::shared_ptr<const PIXMAP> input = data.get(params.a_inPixmap);

	if(input == nullptr) {
		result.addError("No input pixmap");
		data.set(params.a_outPixmap, input);
	}

	else {
		float x = 0.0f;
		float y = 0.0f;

		float r = 0.0f;
		float g = 0.0f;
		float b = 0.0f;

		const possumwood::ExprSymbols& symbols = data.get(params.a_symbols);

		exprtk::symbol_table<float> symbol_table;

		for(auto& s : symbols)
			symbol_table.add_constant(s.first, s.second);

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

		std::string err;
		bool success = false;
		{
			exprtk::parser<float> parser;
			success = parser.compile(data.get(params.a_src), expression);

			if(!success)
				err = parser.error();
		}

		auto cexpr = (const exprtk::expression<float>)expression;

		// compilation failed
		if(!success) {
			result.addError(err);

			// output nothing
			data.set(params.a_outPixmap, std::shared_ptr<const PIXMAP>());
		}

		// compilation success
		else {
			std::unique_ptr<PIXMAP> out(new PIXMAP(input->width(), input->height()));

			// iterate over pixels
			for(std::size_t yi = 0; yi < input->height(); ++yi)
				for(std::size_t xi = 0; xi < input->width(); ++xi) {
					x = xi;
					y = yi;

					// r, g, b variables are changeable from the expression
					std::tie(r,g,b) = readValue((*input)(xi, yi).value());

					// evaluate the expression, ignoring its output
					cexpr.value();

					// write the values back
					writeValue((*out)(xi, yi), std::tie(r,g,b));
				}

			data.set(params.a_outPixmap, std::shared_ptr<const PIXMAP>(out.release()));
		}
	}

	return result;
}

template<typename PIXMAP>
void init(possumwood::Metadata& meta, Params<PIXMAP>& params) {
	meta.addAttribute(params.a_src, "source", std::string("r := x / width;\ng := y / height;\nb := max(0, 1-r-g);"));
	meta.addAttribute(params.a_symbols, "symbols");
	meta.addAttribute(params.a_inPixmap, "in_image");
	meta.addAttribute(params.a_outPixmap, "out_image");

	meta.addInfluence(params.a_src, params.a_outPixmap);
	meta.addInfluence(params.a_symbols, params.a_outPixmap);
	meta.addInfluence(params.a_inPixmap, params.a_outPixmap);

	meta.setCompute([&params](dependency_graph::Values& data) {
		return compute<PIXMAP>(data, params);
	});
	meta.setEditorFactory([&params]() {
		return std::unique_ptr<possumwood::Editor>(new Editor<PIXMAP>(params));
	});
}

possumwood::NodeImplementation s_impl("images/per_pixel_expr", [](possumwood::Metadata& meta) {
	init<possumwood::LDRPixmap>(meta, s_ldrParams);
});

possumwood::NodeImplementation s_impl_hdr("images/per_pixel_expr_hdr", [](possumwood::Metadata& meta) {
	init<possumwood::HDRPixmap>(meta, s_hdrParams);
});

}
