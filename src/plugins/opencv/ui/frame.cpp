#include "opencv/ui/frame.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QFileDialog>

#include <opencv2/opencv.hpp>
#include <OpenImageIO/imageio.h>

#include <boost/algorithm/string/replace.hpp>

namespace {

OIIO::TypeDesc::BASETYPE type(int cvType) {
	switch(cvType) {
		case CV_8U: return OIIO::TypeDesc::UINT8;
		case CV_8S: return OIIO::TypeDesc::INT8;
		case CV_16U: return OIIO::TypeDesc::UINT16;
		case CV_16S: return OIIO::TypeDesc::INT16;
		case CV_32S: return OIIO::TypeDesc::INT32;
		case CV_32F: return OIIO::TypeDesc::FLOAT;
		case CV_64F: return OIIO::TypeDesc::DOUBLE;
		default: throw std::runtime_error("Unknown OpenCV type passed to OIIO");
	}

	return OIIO::TypeDesc::NONE;
}

}

FrameUI::FrameUI() {
	m_widget = new QWidget(nullptr);

	QHBoxLayout* layout = new QHBoxLayout(m_widget);
	layout->setContentsMargins(0, 0, 0, 0);

	m_label = new QLabel(m_widget);
	layout->addWidget(m_label, 1);

	QToolButton* saveButton = new QToolButton();
	saveButton->setIcon(QIcon(":icons/filesave.png"));
	layout->addWidget(saveButton);

	QObject::connect(
		saveButton,
		&QToolButton::pressed,
		[this]() -> void {
			std::string formats = OIIO::get_string_attribute ("extension_list");

			boost::replace_all(formats, ":",  " (*.");
			boost::replace_all(formats, ";",  ");;");
			boost::replace_all(formats, ",",  " *.");
			formats = formats + ")";

			QString selectedFilter = "png (*.png)";
			QString qfilename = QFileDialog::getSaveFileName(m_widget, "Save image", "", formats.c_str(), &selectedFilter);

			if(!qfilename.isEmpty()) {
				// convert from BGR to RGB
				cv::Mat rgb;
				cv::cvtColor(m_value, rgb, cv::COLOR_BGR2RGB);

				std::unique_ptr<OIIO::ImageOutput> out(OIIO::ImageOutput::create(qfilename.toStdString()));
				if(!out) {
					std::stringstream err;
					err << "Could not create an ImageOutput for " << qfilename.toStdString() << ", error = " << OIIO::geterror();
					throw std::runtime_error(err.str());
				}

				OIIO::ImageSpec spec(rgb.cols, rgb.rows, rgb.channels(), type(rgb.depth()));
				if(!out->open(qfilename.toStdString(), spec)) {
					std::stringstream err;
					err << "Could not open " << qfilename.toStdString() << ", error = " << out->geterror();
					throw std::runtime_error(err.str());
				}

				for(int y=0; y<rgb.rows; ++y) {
					if(!out->write_scanline(y, 0, type(rgb.depth()), rgb.ptr(y))) {
						std::stringstream err;
						err << "Could not write pixels to " << qfilename.toStdString() << ", error = " << out->geterror();
						throw std::runtime_error(err.str());
					}
				}

				if(!out->close()) {
					std::stringstream err;
					err << "Error closing " << qfilename.toStdString() << ", error = " << out->geterror();
					throw std::runtime_error(err.str());
				}
			}
		}
	);
}

FrameUI::~FrameUI() {
}

void FrameUI::get(possumwood::opencv::Frame& value) const {
	// do nothing, for now
}

void FrameUI::set(const possumwood::opencv::Frame& value) {
	bool block = m_label->blockSignals(true);

	// update the label
	std::stringstream ss;
	ss << value;

	m_label->setText(ss.str().c_str());

	m_value = *value;

	m_label->blockSignals(block);
}

QWidget* FrameUI::widget() {
	return m_widget;
}
