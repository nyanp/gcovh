#include "gcovh.h"
#include <iostream>
#include <vector>
#include <map>
#include <sstream>

void howto(void) {
	std::cout << "gcovr - gcov html report generator" << std::endl;
	std::cout << " gcov input-files" << std::endl;
}

using gcovh::coverage_data;

typedef std::vector<coverage_data> sources_t;

int main (int argc, char *argv[]) {
	if (argc < 2) {
		std::cout << "error: invalid arg" << std::endl;
		return -1;
	}

	try {
		sources_t sources;

		for (int i = 1; i < argc; i++) {
			sources.push_back(gcovh::parse(argv[i]));
		}

		for (sources_t::iterator it = sources.begin(), end = sources.end(); it != end; ++it) {
			gcovh::generate_coverage_report((*it));
		}
		gcovh::generate_coverage_summary(sources);
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		exit(-1);
	}
}