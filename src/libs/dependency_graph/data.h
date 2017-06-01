#pragma once

namespace dependency_graph {

struct BaseData {
	virtual ~BaseData() {};

	virtual void assign(const BaseData& src) = 0;
	virtual bool isEqual(const BaseData& src) const = 0;

	virtual void toJson(io::json& j) const = 0;
	virtual void fromJson(const io::json& j) = 0;
};

template<typename T>
struct Data : public BaseData {
	public:
		Data(const T& v = T()) : value(v) {};
		virtual ~Data() {};
		virtual void assign(const BaseData& src) override;
		virtual bool isEqual(const BaseData& src) const;

		virtual void toJson(io::json& j) const override;
		virtual void fromJson(const io::json& j) override;

		T value;
};

}
