#include "common.h"

namespace std {

ostream& operator << (ostream& out, const type_info& t) {
	out << t.name();

	return out;
}

}

/////////////

namespace {
	static unsigned s_counter = 0;
}

TestStruct::TestStruct() : id(s_counter++) {
}

bool TestStruct::operator == (const TestStruct& ts) const {
	return id == ts.id;
}

bool TestStruct::operator != (const TestStruct& ts) const {
	return id != ts.id;
}

std::ostream& operator << (std::ostream& out, const TestStruct& t) {
	out << t.id;

	return out;
}
