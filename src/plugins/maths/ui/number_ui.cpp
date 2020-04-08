#include "number_ui.h"

#include <QDoubleSpinBox>

template<typename T>
NumberUI<T>::NumberUI() : m_ui(new typename NumberTraits<T>::QtType(NULL)) {
	m_valueChangeConnection = QObject::connect(
		m_ui,
		static_cast<void(NumberTraits<T>::QtType::*)(typename NumberTraits<T>::CallbackResult)>(&NumberTraits<T>::QtType::valueChanged),
		[this]() -> void {
			this->callValueChangedCallbacks();
		}
	);

	m_ui->setKeyboardTracking(false);

	NumberTraits<T>::init(m_ui);
}

template<typename T>
NumberUI<T>::~NumberUI() {
	QObject::disconnect(m_valueChangeConnection);
}

template<typename T>
void NumberUI<T>::get(typename NumberTraits<T>::Value& value) const {
	value = m_ui->value();
}

template<typename T>
void NumberUI<T>::set(const typename NumberTraits<T>::Value& value) {
	bool block = m_ui->blockSignals(true);

	if(T(m_ui->value()) != value)
		m_ui->setValue(value);

	m_ui->blockSignals(block);
}

template<typename T>
QWidget* NumberUI<T>::widget() {
	return m_ui;
}

template<typename T>
void NumberUI<T>::onFlagsChanged(unsigned flags) {
	m_ui->setReadOnly(flags & possumwood::properties::property_base::kOutput);
	m_ui->setDisabled(flags & possumwood::properties::property_base::kDisabled);
}

///////////////////

template<typename T>
struct NumberTraits {
	typedef T Value;
	typedef QSpinBox QtType;
	typedef int CallbackResult;

	static void init(QSpinBox* ptr) {
		ptr->setRange(std::numeric_limits<T>::min(), std::numeric_limits<int>::max());
	}
};

template<>
struct NumberTraits<float> {
	typedef float Value;
	typedef QDoubleSpinBox QtType;
	typedef double CallbackResult;

	static void init(QDoubleSpinBox* ptr) {
		ptr->setRange(-1e13, 1e13);
		ptr->setDecimals(5);
	}
};

template<>
struct NumberTraits<double> {
	typedef double Value;
	typedef QDoubleSpinBox QtType;
	typedef double CallbackResult;

	static void init(QDoubleSpinBox* ptr) {
		ptr->setRange(-1e13, 1e13);
		ptr->setDecimals(5);
	}
};

///////////////////

template class NumberUI<unsigned char>;
template class NumberUI<char>;

template class NumberUI<unsigned short>;
template class NumberUI<short>;

template class NumberUI<unsigned>;
template class NumberUI<int>;

template class NumberUI<float>;
template class NumberUI<double>;
