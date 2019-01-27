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

dependency_graph::InAttr<std::string> a_src;
dependency_graph::InAttr<possumwood::ExprSymbols> a_symbols;
dependency_graph::InAttr<std::shared_ptr<const possumwood::Pixmap>> a_inPixmap;
dependency_graph::OutAttr<std::shared_ptr<const possumwood::Pixmap>> a_outPixmap;

class Popup : public QMenu {
	public:
		Popup(QWidget* parent) : QMenu(parent) {
		}

		void addItem(const std::string& name) {
			addAction(name.c_str());
		}

	private:
};


class Editor : public possumwood::SourceEditor {
	public:
		Editor() : SourceEditor(a_src), m_popup(nullptr) {
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
			if(attr == a_symbols) {
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
			for(auto& s : values().get(a_symbols)) {
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
		QPushButton* m_varsButton;

		Popup* m_popup;
};

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	std::shared_ptr<const possumwood::Pixmap> input = data.get(a_inPixmap);

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

		const possumwood::ExprSymbols& symbols = data.get(a_symbols);

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
			success = parser.compile(data.get(a_src), expression);

			if(!success)
				err = parser.error();
		}

		auto cexpr = (const exprtk::expression<float>)expression;

		// compilation failed
		if(!success) {
			result.addError(err);

			// output nothing
			data.set(a_outPixmap, std::shared_ptr<const possumwood::Pixmap>());
		}

		// compilation success
		else {
			std::unique_ptr<possumwood::Pixmap> out(new possumwood::Pixmap(input->width(), input->height()));

			// iterate over pixels
			for(std::size_t yi = 0; yi < input->height(); ++yi)
				for(std::size_t xi = 0; xi < input->width(); ++xi) {
					x = xi;
					y = yi;

					// r, g, b variables are changeable from the expression
					auto value = (*input)(xi, yi).value();

					r = (float)value[0] / 255.0f;
					g = (float)value[1] / 255.0f;
					b = (float)value[2] / 255.0f;

					// evaluate the expression, ignoring its output
					cexpr.value();

					(*out)(xi, yi) = possumwood::Pixel::value_t{{
						possumwood::Pixmap::channel_t(std::min(255.0f, std::max(0.0f, r * 255.0f))),
						possumwood::Pixmap::channel_t(std::min(255.0f, std::max(0.0f, g * 255.0f))),
						possumwood::Pixmap::channel_t(std::min(255.0f, std::max(0.0f, b * 255.0f))),
					}};
				}

			data.set(a_outPixmap, std::shared_ptr<const possumwood::Pixmap>(out.release()));
		}
	}

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_src, "source", std::string("r := x / width;\ng := y / height;\nb := max(0, 1-r-g);"));
	meta.addAttribute(a_symbols, "symbols");
	meta.addAttribute(a_inPixmap, "in_image");
	meta.addAttribute(a_outPixmap, "out_image");

	meta.addInfluence(a_src, a_outPixmap);
	meta.addInfluence(a_symbols, a_outPixmap);
	meta.addInfluence(a_inPixmap, a_outPixmap);

	meta.setCompute(&compute);
	meta.setEditor<Editor>();
}

possumwood::NodeImplementation s_impl("images/per_pixel_expr", init);

}
