#pragma once

#include <memory>
#include <string>
#include <typeinfo>

#include <luabind/luabind.hpp>

namespace possumwood { namespace lua {

class State;

class Variable final {
	public:
		template<typename T>
		Variable(const std::string& name, const T& value);
		~Variable() = default;

		const std::type_info& type() const;

		const std::string& name() const;
		bool operator == (const Variable& var) const;
		bool operator != (const Variable& var) const;

		void init(State& s) const;
		std::string str() const;

	private:
		struct HolderBase {
			HolderBase() = default;
			virtual ~HolderBase() = default;

			HolderBase(const HolderBase&) = delete;
			HolderBase& operator = (const HolderBase&) = delete;

			virtual const std::type_info& type() const = 0;

			virtual void init(State& s, const std::string& name) const = 0;
			virtual std::string str() const = 0;

			virtual bool equalTo(const HolderBase& v) const = 0;
		};

		template<typename T>
		class Holder : public HolderBase {
			public:
				Holder(const T& value);

				virtual const std::type_info& type() const override;

				virtual void init(State& s, const std::string& name) const override;
				virtual std::string str() const override;

				virtual bool equalTo(const HolderBase& v) const override;

			private:
				T m_value;
		};

		std::string m_name;
		std::shared_ptr<const HolderBase> m_value;
};

std::ostream& operator << (std::ostream& out, const Variable& v);

}}
