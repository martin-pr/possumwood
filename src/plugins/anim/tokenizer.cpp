#include "tokenizer.h"

#include <cassert>

using std::cout;
using std::endl;

namespace anim {

Tokenizer::Tokenizer(std::istream& in) : m_input(in), m_active(NULL), m_line(1) {
}

bool Tokenizer::eof() const {
	return m_input.eof() || (!m_input.good());
}

const Tokenizer::Token& Tokenizer::current() const {
	return m_current;
}

const Tokenizer::Token& Tokenizer::next() {
	// no further items in the m_future queue - run the parsers loop to get some
	while(!eof() && m_future.empty()) {
		// add a new value to the future queue
		m_future.push(Token());

		// loop until emit is called, or until the end of file 
		// (emit makes a new m_future item)
		assert(m_active != NULL);
		do {
			// process next-to-be-read character
			// (accept() / reject() call get() to move on the next character;
			// allows to switch states without reading anything)
			m_active->m_parse(m_input.peek());
		}
		while(m_future.back().valid && (m_future.size() == 1) && (!eof()));

		// cull out any remaining empty tokens that might appear when parser didn't produce
		//   any token
		while(!m_future.empty() && !m_future.front().valid)
			m_future.pop();
	}

	// the queue of parsed tokens is not empty - use next future token
	if(!m_future.empty()) {
		m_current = m_future.front();
		
		// get rid of current value and any possible empty tokens
		m_future.pop();
		while(!m_future.empty() && !m_future.front().valid)
			m_future.pop();
	}
	// else eof had to happen, because otherwise the loop above would produce a token
	else {
		assert(eof());
		m_current.value = "";
	}

	return m_current;
}

void Tokenizer::emit(bool acceptEmptyTokens) {
	// emit only non-empty tokens
	if(!m_future.back().value.empty() || acceptEmptyTokens) {
		if(!m_future.back().valid)
			m_future.back().line = m_line;
		m_future.back().valid = true;

		m_future.push(Token());
	}
}

void Tokenizer::accept(char c) {
	// remember the line the token started on
	if(!m_future.back().valid)
		m_future.back().line = m_line;
	m_future.back().valid = true;
	
	// add the character to the future token
	m_future.back().value += c;

	// and skip to the next character
	reject();
}

void Tokenizer::accept(const std::string& s) {
	// remember the line the token started on
	if(!m_future.back().valid)
		m_future.back().line = m_line;
	m_future.back().valid = true;
	
	// add the character to the future token
	m_future.back().value += s;

	// and skip to the next character
	reject();
}

void Tokenizer::reject() {
	// and read next character, counting the lines in the process
	if(m_input.get() == '\n')
		++m_line;
}

const std::string Tokenizer::currentToken() const {
	assert(!m_future.empty());
	return m_future.back().value;
}

/////

Tokenizer::State::State(Tokenizer* parent) : m_parent(parent) {
}

Tokenizer::State& Tokenizer::State::operator = (const std::function<void(char)>& parser) {
	m_parse = parser;

	return *this;
}

void Tokenizer::State::setActive() {
	assert(m_parent != NULL);
	m_parent->m_active = this;
}

}