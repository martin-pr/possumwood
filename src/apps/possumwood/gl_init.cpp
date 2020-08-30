#include "gl_init.h"

#include <GL/glew.h>

#include <cassert>
#include <iostream>

void doInitGlew() {
	glewExperimental = true;
	auto glewErr = glewInit();

	if(glewErr != GLEW_OK) {
		std::cout << "Error initialising GLEW - " << glewGetErrorString(glewErr) << std::endl;
		exit(1);
	}

	assert(glGenVertexArrays != NULL);
}
