#include "om_log.h"

OMLog::OMLog() {
	m_info.fn = [this](const std::string& s) {
		m_state.addInfo(s);
		std::cout << "(i) " << s << std::endl;
	};
	omlog().connect(m_info);

	m_warn.fn = [this](const std::string& s) {
		m_state.addWarning(s);
		std::cout << "(w) " << s << std::endl;
	};
	omout().connect(m_warn);

	m_err.fn = [this](const std::string& s) {
		m_state.addError(s);
		std::cout << "(e) " << s << std::endl;
	};
	omerr().connect(m_err);
}

OMLog::~OMLog() {
	omlog().disconnect(m_info);
	omout().disconnect(m_warn);
	omerr().disconnect(m_err);
}

dependency_graph::State& OMLog::state() {
	return m_state;
}

