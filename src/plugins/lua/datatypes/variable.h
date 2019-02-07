#pragma once

#include <memory>
#include <string>
#include <typeinfo>

#include <luabind/luabind.hpp>

namespace possumwood { namespace lua {

class State;

class Variable {
	public:
		Variable(const std::string& name);
		virtual ~Variable() = default;

		virtual std::unique_ptr<Variable> clone() const = 0;
		virtual const std::type_info& type() const = 0;

		const std::string& name() const;
		bool operator == (const Variable& var) const;
		bool operator != (const Variable& var) const;

		virtual void init(State& s) = 0;

	protected:
		Variable(const Variable& con);
		Variable& operator = (const Variable& con);

		virtual bool equalTo(const Variable& v) const = 0;

	private:
		std::string m_name;
};

template<typename T>
class PODVariable final : public Variable {
	public:
		PODVariable(const std::string& name, const T& value);

		virtual std::unique_ptr<Variable> clone() const override;
		virtual const std::type_info& type() const override;

		virtual void init(State& s) override;

	protected:
		virtual bool equalTo(const Variable& v) const override;

	private:
		T m_value;
};

}}
