#pragma once

#include <string>

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>

#include "port.h"

namespace node_editor {

class Node : public QGraphicsItem {
  public:
	enum State { kOk = 0, kInfo, kWarning, kError };

	struct PortDefinition {
		PortDefinition(const QString& n, const Port::Type& t = Port::Type::kInput,
		               const QColor& c = QColor(255, 255, 255),
		               const Port::Orientation& o = Port::Orientation::kHorizontal, bool vis = true)
		    : name(n), type(t), color(c), orientation(o), visible(vis) {
		}

		QString name;
		Port::Type type;
		QColor color;
		Port::Orientation orientation;
		bool visible;
	};

	Node(const QString& name, const QPointF& position = QPointF(0, 0), const QColor& color = QColor(64, 64, 64));
	virtual ~Node();

	const QString name() const;
	void setName(const QString& name);

	unsigned portCount() const;
	Port& port(unsigned i);

	void addPort(const PortDefinition& def);
	void removePort(Port& p);

	void setState(const State& s);
	const State& state() const;

	virtual QRectF boundingRect() const override;

	bool isReadOnly() const;
	void setReadOnly(bool ro);

  protected:
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;

  private:
	void updateRect();

	QGraphicsRectItem* m_titleBackground;
	QGraphicsTextItem* m_title;
	QVector<Port*> m_ports;

	State m_state;

	QColor m_color;
	QRectF m_rect;
};

}  // namespace node_editor
