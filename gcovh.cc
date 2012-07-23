#include "gcovh.h"
#include <iostream>

void howto(void) {
	std::cout << "gcovr - gcov html report generator" << std::endl;
	std::cout << " gcov input-files" << std::endl;
}


int main (int argc, char *argv[]) {
	if (argc < 2) {
		std::cout << "error: invalid arg" << std::endl;
		return -1;
	}

	for (int i = 1; i < argc; i++) {
		gcovh::parsed_source content = gcovh::parse(argv[i]);
		gcovh::write(content, std::string(argv[i]) + ".html");
	}

}