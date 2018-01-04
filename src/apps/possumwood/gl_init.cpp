#include "gl_init.h"

#include <iostream>
#include <cassert>

#include <GL/glew.h>

void doInitGlew() {
	glewExperimental = true;
	auto glewErr = glewInit();

	if(glewErr != GLEW_OK) {
		std::cout << "Error initialising GLEW - " << glewGetErrorString(glewErr)
		          << std::endl;
		exit(1);
	}

	assert(glGenVertexArrays != NULL);
}
