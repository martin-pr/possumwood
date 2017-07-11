#pragma once

#include <string>
#include <vector>

namespace dependency_graph {

class State {
	public:
		enum MessageType {
			kInfo = 0,
			kWarning = 1,
			kError = 2
		};

		State();

		void addInfo(const std::string& info);
		void addWarning(const std::string& warn);
		void addError(const std::string& err);

		bool errored() const;

		typedef std::vector<std::pair<MessageType, std::string>>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

	protected:
	private:
		std::vector<std::pair<MessageType, std::string>> m_messages;
		bool m_errored;
};

}
