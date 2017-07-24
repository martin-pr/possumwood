#pragma once

#include <set>

#include <boost/noncopyable.hpp>
#include <boost/variant.hpp>
#include <boost/signals2.hpp>

namespace possumwood {

class App;

/// A simple configuration container, allowing to store stringly-typed key-value pairs.
/// The same container type is used for storing application-wide configuration and scene-wide one.
/// For the time being, the initialisation is done in derived-class constructor.
class Config : public boost::noncopyable {
	public:
		class Item {
			public:
				enum Flags {
					kNoFlags = 0,
					kNotUserAccessible = 1 //< non-UI properties (e.g., scene windows setup)
				};

				template<typename T>
				Item(const std::string& name, const std::string& group, const T& defaultValue, const Flags& f, const std::string& description);

				Item(const Item& i);
				Item& operator = (const Item& i);

				const std::string& name() const;

				const std::string& group() const;

				const std::string type() const;

				const Flags& flags() const;

				template<typename T>
				const T& defaultValue() const;

				const std::string& description() const;

				template<typename T>
				bool is() const;

				template<typename T>
				const T& as() const;

				template<typename T>
				Item& operator = (const T& val);

				boost::signals2::connection onChanged(std::function<void(Item&)> callback);

			private:
				std::string m_name, m_group, m_description;
				Flags m_flags;
				boost::variant<int, float, std::string> m_value, m_defaultValue;
				boost::signals2::signal<void(Item&)> m_onChanged;

				friend class Config;
		};

		virtual ~Config();

		Item& operator[](const std::string& name);
		const Item& operator[](const std::string& name) const;

		/// resets the Config items to their original values
		void reset();

		typedef std::vector<Item>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		typedef std::vector<Item>::iterator iterator;
		iterator begin();
		iterator end();

	protected:
		void addItem(const Item& i);

	private:
		std::vector<Item> m_items;

	friend class App;
};

}
