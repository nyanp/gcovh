#include "gcovh.h"

int main (int argc, char *argv[]) {
	if (argc != 2) {
		std::cout << "error: invalid arg" << std::endl;
		return -1;
	}
	gcovh::parsed_source content = gcovh::parse(argv[1]);
	gcovh::write(content, std::string(argv[1]) + ".html");
}