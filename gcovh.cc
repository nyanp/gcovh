#include "gcovh.h"
#include <iostream>
#include <vector>
#include <map>

void howto(void) {
	std::cout << "gcovr - gcov html report generator" << std::endl;
	std::cout << " gcov input-files" << std::endl;
}


typedef std::map<std::string, gcovh::parsed_source> sources_t;

int main (int argc, char *argv[]) {
	if (argc < 2) {
		std::cout << "error: invalid arg" << std::endl;
		return -1;
	}

	sources_t sources;

	for (int i = 1; i < argc; i++) {
		sources[argv[i]] = gcovh::parse(argv[i]);
	}

	for (sources_t::iterator it = sources.begin(), end = sources.end(); it != end; ++it) {
		gcovh::write((*it).second, (*it).first + ".html");
	}

}