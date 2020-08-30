#include "rtti.h"

#include <cxxabi.h>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <map>

namespace dependency_graph {

namespace {

/// Unmangling buffer class, keeping buffer memory allocated using C malloc/free (intended to be used$
struct UnmanglingBuffer {
	UnmanglingBuffer(size_t length);
	~UnmanglingBuffer();

	char* m_buffer;
	size_t m_length;
};

UnmanglingBuffer::UnmanglingBuffer(size_t length) : m_length(length) {
	m_buffer = (char*)calloc(length, 1);
}

UnmanglingBuffer::~UnmanglingBuffer() {
	free(m_buffer);
};

}  // namespace

////

const std::string unmangledName(const char* name) {
	static boost::shared_mutex m;

	std::string value = name;

	// a cache of unmangled names
	static std::map<std::string, std::string> s_nameCache;

	{
		// try to find a corresponding item in the cache, and return if found
		boost::shared_lock<boost::shared_mutex> lock(m);

		std::map<std::string, std::string>::const_iterator i = s_nameCache.find(value);
		if(i != s_nameCache.end())
			return i->second;
	}

	// not found
	// lock this method - can possibly be called from multiple threads at the same time
	boost::unique_lock<boost::shared_mutex> lock(m);

	static UnmanglingBuffer buffer(2048);
	int status = 0;

	// attempt to unmangle the input
	char* tmp = abi::__cxa_demangle(name, buffer.m_buffer, &buffer.m_length, &status);
	if(tmp != NULL)
		buffer.m_buffer = tmp;

	// return unmangled string on success, and input string on failure
	if(status == 0)
		value = buffer.m_buffer;

	// add to the cache
	s_nameCache[name] = value;

	// and return
	return value;
}

}  // namespace dependency_graph
