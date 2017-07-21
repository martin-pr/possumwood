#pragma once

#include <iostream>
#include <functional>
#include <queue>

#include <boost/noncopyable.hpp>

#include "lexical_cast.h"

namespace anim {

/// a passive tokenizer class built on std::functions to do the main work - makes the code C++-like
/// but still really simple and short. Its not too powerful, though, about as much as regular languages
/// (which is kinda normal for tokenizers, discounting C++).
class Tokenizer : public boost::noncopyable {
	public:
		struct Token {
			bool operator == (const std::string& v) const {
				return value == v;
			}

			bool operator != (const std::string& v) const {
				return value != v;
			}

			unsigned line;
			std::string value;
			bool valid = false;
		};

		Tokenizer(std::istream& in);
		
		// true if nothing more is to be read
		bool eof() const;
		
		// reads a next token and returns it
		const Token& next();
		// returns the current token
		const Token& current() const;
	
	protected:
		class State final : public boost::noncopyable {
			public:
				State(Tokenizer* parent);
				
				State& operator = (const std::function<void(char)>& parser);

				void setActive();
				
			private:
				Tokenizer* m_parent;
				std::function<void(char)> m_parse;

			friend class Tokenizer;
		};
		
		// emit current token (on exit, of course)
		void emit(bool acceptEmptyTokens = false);
		
		// accepts a character (can be case-converted if needed)
		void accept(char c);
		// accepts a string (multiple characters at the same time - usable for hash-bangs)
		void accept(const std::string& s);
		// skips current character
		void reject();

		// returns the currently processed token
		const std::string currentToken() const;
	
	private:
		std::istream& m_input;

		State* m_active;
		Token m_current;
		std::queue<Token> m_future;
		unsigned m_line;

	friend class State;
};

/// reading operator for a value
template<typename T>
Tokenizer& operator >> (Tokenizer& t, T& val) {
	val = lexical_cast<T>(t.next().value);

	return t;
}

/// reading operator for a string (doesn't have to be cast)
inline Tokenizer& operator >> (Tokenizer& t, std::string& val) {
	val = t.next().value;

	return t;
}

/// a reading operator for reading structure tokens
inline Tokenizer& operator >> (Tokenizer& t, const char* val) {
	if(t.next().value != val)
		throw std::runtime_error(std::string("expected '") + val + "', but found '" + t.current().value + "'");

	return t;
}

}