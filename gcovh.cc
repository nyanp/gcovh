/*
    Copyright (c) 2013, Taiga Nomi
	All rights reserved.
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.
	* Neither the name of the <organization> nor the
	names of its contributors may be used to endorse or promote products
	derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY 
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
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
		std::cerr << "error: invalid arg" << std::endl;
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
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		exit(-1);
	} catch (...) {
		std::cout << "unknown error" << std::endl;
		exit(-1);
	}
}