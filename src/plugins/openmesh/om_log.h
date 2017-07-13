#pragma once

#include <functional>

#include <boost/noncopyable.hpp>

#include <OpenMesh/Core/System/omstream.hh>

#include <dependency_graph/state.h>

/// A simple class for scoped log redirection.
/// On construction, redirects the openmesh errors/warnings/messages to an instance
/// of State class.
class OMLog : public boost::noncopyable {
	public:
		OMLog();
		~OMLog();

		dependency_graph::State& state();

	protected:
	private:
		/// just a simple class converting << operator to a functor call
		struct Target {
			void operator << (const std::string& s) { fn(s); }

			std::function<void(const std::string&)> fn;
		};

		dependency_graph::State m_state;

		Target m_info, m_warn, m_err;
};
