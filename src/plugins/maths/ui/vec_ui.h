#pragma once

#include <array>

#include <QMetaObject>

#include <possumwood_sdk/properties/property.h>

template<typename T>
struct VecTraits;

template<typename T>
class VecUI : public possumwood::properties::property<T, VecUI<T>> {
	public:
		VecUI();
		virtual ~VecUI();

		virtual void get(T& value) const override;
		virtual void set(const T& value) override;

		virtual QWidget* widget() override;

	protected:
		virtual void onFlagsChanged(unsigned flags) override;

	private:
		QWidget* m_widget;
		std::array<typename VecTraits<T>::QtType*, VecTraits<T>::dims()> m_values;
		std::array<QMetaObject::Connection, VecTraits<T>::dims()> m_connections;
};
