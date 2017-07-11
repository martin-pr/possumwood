#include "state.h"

namespace dependency_graph {

State::State() : m_errored(false) {
}

void State::addInfo(const std::string& info) {
	m_messages.push_back(std::make_pair(kInfo, info));
}

void State::addWarning(const std::string& warn) {
	m_messages.push_back(std::make_pair(kWarning, warn));
}

void State::addError(const std::string& err) {
	m_messages.push_back(std::make_pair(kError, err));
	m_errored = true;
}

bool State::errored() const {
	return m_errored;
}

State::const_iterator State::begin() const {
	return m_messages.begin();
}

State::const_iterator State::end() const {
	return m_messages.end();
}

bool State::operator == (const State& s) const {
	return m_errored == s.m_errored && m_messages == s.m_messages;
}

bool State::operator != (const State& s) const {
	return m_errored != s.m_errored || m_messages == s.m_messages;
}

}
