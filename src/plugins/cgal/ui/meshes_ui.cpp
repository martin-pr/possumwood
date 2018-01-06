// #include "meshes_ui.h"

// #include <QHeaderView>
// #include <QHBoxLayout>
// #include <QSizePolicy>

// MeshesUI::MeshesUI() {
// 	m_widget = new QWidget();
// 	QHBoxLayout* layout = new QHBoxLayout(m_widget);
// 	layout->setContentsMargins(0, 0, 0, 0);

// 	m_meshCountLabel = new QLabel();
// 	layout->addWidget(m_meshCountLabel, 1);

// 	m_showDetailsButton = new QPushButton("Show details...");
// 	layout->addWidget(m_showDetailsButton, 0);
// 	QObject::connect(m_showDetailsButton, &QPushButton::pressed,
// 	                 [this]() { m_detailsDialog->show(); });

// 	m_detailsDialog = new QDialog(m_widget);
// 	m_detailsDialog->hide();
// 	m_detailsDialog->resize(1000, 800);
// 	QHBoxLayout* dialogLayout = new QHBoxLayout(m_detailsDialog);
// 	dialogLayout->setContentsMargins(0, 0, 0, 0);

// 	m_detailsWidget = new QTableWidget();
// 	dialogLayout->addWidget(m_detailsWidget);
// }

// MeshesUI::~MeshesUI() {
// }

// void MeshesUI::get(possumwood::Meshes& value) const {
// 	assert(false &&
// 	       "get() on a mesh should never be called - UI change signals are not "
// 	       "implemented");
// }

// void MeshesUI::set(const possumwood::Meshes& value) {
// 	m_detailsWidget->clear();

// 	m_detailsWidget->setWordWrap(true);

// 	m_detailsWidget->setColumnCount(7);
// 	m_detailsWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Name"));
// 	m_detailsWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Halfedges"));
// 	m_detailsWidget->setHorizontalHeaderItem(2, new QTableWidgetItem("Vertices"));
// 	m_detailsWidget->setHorizontalHeaderItem(3, new QTableWidgetItem("Faces"));
// 	m_detailsWidget->setHorizontalHeaderItem(4, new QTableWidgetItem("Halfedge props"));
// 	m_detailsWidget->setHorizontalHeaderItem(5, new QTableWidgetItem("Vertex props"));
// 	m_detailsWidget->setHorizontalHeaderItem(6, new QTableWidgetItem("Face props"));

// 	m_detailsWidget->horizontalHeader()->setStretchLastSection(false);
// 	for(unsigned a = 0; a < 7; ++a)
// 		m_detailsWidget->horizontalHeader()->setSectionResizeMode(
// 		    a, QHeaderView::ResizeToContents);

// 	m_detailsWidget->setRowCount(value.size());

// 	unsigned counter = 0;
// 	for(auto& m : value) {
// 		m_detailsWidget->setItem(counter, 0, new QTableWidgetItem(m.name().c_str()));

// 		m_detailsWidget->setItem(
// 		    counter, 1,
// 		    new QTableWidgetItem(std::to_string(m.mesh().num_faces()).c_str()));
// 		m_detailsWidget->setItem(
// 		    counter, 2,
// 		    new QTableWidgetItem(std::to_string(m.mesh().num_vertices()).c_str()));
// 		m_detailsWidget->setItem(
// 		    counter, 3,
// 		    new QTableWidgetItem(std::to_string(m.mesh().num_halfedges()).c_str()));

// 		auto hepv = m.mesh().properties<possumwood::CGALPolyhedron::Halfedge_index>();
// 		const std::string hep = boost::algorithm::join(hepv, " ");
// 		m_detailsWidget->setItem(counter, 4, new QTableWidgetItem(hep.c_str()));

// 		auto vpv = m.mesh().properties<possumwood::CGALPolyhedron::Vertex_index>();
// 		const std::string vp = boost::algorithm::join(vpv, " ");
// 		m_detailsWidget->setItem(counter, 5, new QTableWidgetItem(vp.c_str()));

// 		auto fpv = m.mesh().properties<possumwood::CGALPolyhedron::Face_index>();
// 		const std::string fp = boost::algorithm::join(fpv, " ");
// 		m_detailsWidget->setItem(counter, 6, new QTableWidgetItem(fp.c_str()));

// 		++counter;
// 	}

// 	for(int a = 0; a < m_detailsWidget->columnCount(); ++a)
// 		for(int b = 0; b < m_detailsWidget->rowCount(); ++b)
// 			m_detailsWidget->item(b, a)->setFlags(Qt::ItemIsSelectable |
// 			                                      Qt::ItemIsEnabled);

// 	if(value.empty())
// 		m_meshCountLabel->setText("(empty)");
// 	else
// 		m_meshCountLabel->setText((std::to_string(value.size()) + " mesh(es)").c_str());
// }

// QWidget* MeshesUI::widget() {
// 	return m_widget;
// }
