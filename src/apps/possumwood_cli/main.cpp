#include <iostream>

#include <possumwood_sdk/app.h>

#include "common.h"

int main(int argc, char* argv[]) {
	// create the possumwood application
	possumwood::App papp;

	// load all plugins into an RAII container
	PluginsRAII plugins;

	return 0;
}
