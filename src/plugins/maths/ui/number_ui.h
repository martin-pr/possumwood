#pragma once

#include <QMetaObject>

#include <possumwood_sdk/properties/property.h>

template<typename T>
struct NumberTraits;

template<typename T>
class NumberUI : public possumwood::properties::property<typename NumberTraits<T>::Value, NumberUI<T>> {
	public:
		NumberUI();
		virtual ~NumberUI();

		virtual void get(typename NumberTraits<T>::Value& value) const override;
		virtual void set(const typename NumberTraits<T>::Value& value) override;

		virtual QWidget* widget() override;

	protected:
		virtual void onFlagsChanged(unsigned flags) override;

	private:
		typename NumberTraits<T>::QtType* m_ui;
		QMetaObject::Connection m_valueChangeConnection;
};
