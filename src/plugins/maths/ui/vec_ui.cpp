#include "vec_ui.h"

#include <OpenEXR/ImathVec.h>

#include <QtWidgets/QHBoxLayout>
#include <QDoubleSpinBox>

template<typename T>
VecUI<T>::VecUI() {
	m_widget = new QWidget(NULL);

	m_widget->setFocusPolicy(Qt::StrongFocus);

	QHBoxLayout* layout = new QHBoxLayout(m_widget);
	layout->setContentsMargins(0,0,0,0);

	for(unsigned a=0;a<VecTraits<T>::dims();++a) {
		m_values[a] = new typename VecTraits<T>::QtType(NULL);
		layout->addWidget(m_values[a]);

		m_connections[a] = QObject::connect(
			m_values[a],
			static_cast<void(VecTraits<T>::QtType::*)(typename VecTraits<T>::CallbackResult)>(&VecTraits<T>::QtType::valueChanged),
			[this]() -> void {
				this->callValueChangedCallbacks();
			}
		);

		m_values[a]->setKeyboardTracking(false);

		if(a > 0)
			QWidget::setTabOrder(m_values[a-1], m_values[a]);
	}

	VecTraits<T>::init(m_values);
}

template<typename T>
VecUI<T>::~VecUI() {
	for(auto& c : m_connections)
		QObject::disconnect(c);
}

template<typename T>
void VecUI<T>::get(T& value) const {
	for(unsigned a=0;a<VecTraits<T>::dims();++a)
		value[a] = m_values[a]->value();
}

template<typename T>
void VecUI<T>::set(const T& value) {
	for(auto& v : m_values)
		v->blockSignals(true);

	for(unsigned a=0;a<VecTraits<T>::dims();++a)
		if(typename VecTraits<T>::Value(m_values[a]->value()) != value.getValue()[a])
			m_values[a]->setValue(value.getValue()[a]);

	for(auto& v : m_values)
		v->blockSignals(false);
}

template<typename T>
QWidget* VecUI<T>::widget() {
	return m_widget;
}

template<typename T>
void VecUI<T>::onFlagsChanged(unsigned flags) {
	for(unsigned a=0;a<2;++a) {
		m_values[a]->setReadOnly(flags & possumwood::properties::property_base::kOutput);
		m_values[a]->setDisabled(flags & possumwood::properties::property_base::kDisabled);
	}
}

///////////////////

template<typename T>
struct ElementTraits {
	typedef int CallbackResult;
	typedef QSpinBox QtType;

	static void init(QSpinBox* ptr) {
		ptr->setRange(std::numeric_limits<T>::min(), std::numeric_limits<int>::max());
	}
};

template<>
struct ElementTraits<float> {
	typedef double CallbackResult;
	typedef QDoubleSpinBox QtType;

	static void init(QDoubleSpinBox* ptr) {
		ptr->setRange(-1e13, 1e13);
		ptr->setDecimals(5);
	}
};

template<>
struct ElementTraits<double> {
	typedef double CallbackResult;
	typedef QDoubleSpinBox QtType;

	static void init(QDoubleSpinBox* ptr) {
		ptr->setRange(-1e13, 1e13);
		ptr->setDecimals(5);
	}
};

///////////////////////////

template<typename T>
struct VecTraits;

template<typename T>
struct VecTraits<Imath::Vec2<T>> {
	typedef T Value;
	typedef typename ElementTraits<T>::QtType QtType;
	typedef typename ElementTraits<T>::CallbackResult CallbackResult;
	static constexpr int dims() { return 2; }

	static void init(std::array<QtType*, 2>& arr) {
		ElementTraits<T>::init(arr[0]);
		ElementTraits<T>::init(arr[1]);
	}
};

template<typename T>
struct VecTraits<Imath::Vec3<T>> {
	typedef T Value;
	typedef typename ElementTraits<T>::QtType QtType;
	typedef typename ElementTraits<T>::CallbackResult CallbackResult;
	static constexpr int dims() { return 3; }

	static void init(std::array<QtType*, 3>& arr) {
		ElementTraits<T>::init(arr[0]);
		ElementTraits<T>::init(arr[1]);
		ElementTraits<T>::init(arr[2]);
	}
};

///////////////////

template class VecUI<Imath::V2i>;
template class VecUI<Imath::Vec2<unsigned>>;
template class VecUI<Imath::V2f>;
template class VecUI<Imath::V2d>;

template class VecUI<Imath::V3i>;
template class VecUI<Imath::Vec3<unsigned>>;
template class VecUI<Imath::V3f>;
template class VecUI<Imath::V3d>;
