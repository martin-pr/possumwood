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
dependency_graph::InAttr<std::shared_ptr<const QPixmap>> a_inPixmap;
dependency_graph::OutAttr<std::shared_ptr<const QPixmap>> a_outPixmap;

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
