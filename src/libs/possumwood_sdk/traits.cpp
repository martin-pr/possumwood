#include "traits.h"

// explicit calls for concrete specialisation of numeric traits - will not be necessary
//   one the traits are implemented fully

#pragma GCC diagnostic warning "-Wunused-value"

void fooBar() {
	possumwood::Traits<bool>::io;
	possumwood::Traits<float>::io;
	possumwood::Traits<int>::io;
	possumwood::Traits<unsigned>::io;
}
