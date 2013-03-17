#include "gcovh.h"
#include <iostream>
#include <cmath>

using namespace std;

static int case_id = 0;

template<typename InputStream>
bool equal (InputStream a, InputStream b) {
	return a == b;
}

template<>
bool equal (const char* a, const char* b) {
	return strcmp(a, b) == 0;
}

template<>
bool equal<double> (double a, double b) {
	return std::abs(a - b) < 1E10;
}


template<typename InputStream>
bool check (InputStream result, InputStream expected) {
	cout << "case #" << case_id++ << ":";

	if (equal(expected, result)) 
		cout << "passed" << endl;
	else
		cout << "failed! (expected:" << expected << ", result:" << result << endl; 

	return equal(expected, result);
}

#define TEST(expected, result) { if (!check(expected, result)) return -1; }


int test1(void) {
	std::string s = 
		"        -:    0:Source:foo.c\n"
		"        -:    0:Programs:1\n"
		"        -:    1:#include <stdio.h>\n"
		"        -:    2:\n"
		"        1:    3:int main (void) {\n"
		"        1:    4:    printf(\"foo\");\n"
		"        1:    5:    return 0;\n"
		"        -:    6:}\n";

	gcovh::parser<std::istringstream> p(s);

	gcovh::coverage_data src = p.parse();

	TEST(src.line_coverage(), 100.0);
	TEST(src.lines_executed(), 3);
	TEST(src.lines_total(), 3);
	TEST(src.source_file(), std::string("foo.c"));
	
	gcovh::source_lines lines = src.all();

	TEST((int)lines.size(), 6);
	TEST(lines[0].executable(), false);
	TEST(lines[1].executable(), false);
	TEST(lines[2].executable(), true);
	TEST(lines[3].executable(), true);
	TEST(lines[4].executable(), true);
	TEST(lines[5].executable(), false);

	TEST(lines[0].exec_count(), "-");
	TEST(lines[1].exec_count(), "-");
	TEST(lines[2].exec_count(), "1");
	TEST(lines[3].exec_count(), "1");
	TEST(lines[4].exec_count(), "1");
	TEST(lines[5].exec_count(), "-");

	gcovh::generate_coverage_report(src, "test1.out.html");

	return 0;
}

int test2(void) {
	std::string s = 
		 "        -:    0:Graph:foo.gcno\n"
		 "        -:    0:Data:foo.gcda\n"
		 "      100:    1:int foo (int n) {\n"
		 "      100:    2:    if (n == 0)\n"
		 "    #####:    3:        std::cout << \"foo\" std::endl;\n"
		 "      100:    4:}\n";

	gcovh::parser<std::istringstream> p(s);

	gcovh::coverage_data src = p.parse();

	TEST(src.line_coverage(), 75.0);
	TEST(src.lines_executed(), 3);
	TEST(src.lines_total(), 4);
	TEST(src.graph_file(), std::string("foo.gcno"));
	TEST(src.data_file(), std::string("foo.gcda"));
	
	gcovh::source_lines lines = src.all();

	TEST((int)lines.size(), 4);
	TEST(lines[0].executable(), true);
	TEST(lines[1].executable(), true);
	TEST(lines[2].executable(), true);
	TEST(lines[3].executable(), true);

	TEST(lines[0].exec_count(), "100");
	TEST(lines[1].exec_count(), "100");
	TEST(lines[2].exec_count(), "#####");
	TEST(lines[3].exec_count(), "100");

	gcovh::generate_coverage_report(src, "test2.out.html");

	return 0;
}

int test3(void) {
	std::string s = 
		"function main called 1 returned 100% blocks executed 86%\n"
		"        1:    1:int main(void) {\n"
		"        -:    2:    int i, total;\n"
		"        1:    3:    total = 0;\n"
		"       11:    4:    for (i = 0; i < 10; i++)\n"
		"branch  0 taken 91%\n"
		"branch  1 taken 9% (fallthrough)\n"
		"       10:    5:        total += i;\n"
		"        1:    6:    if (total != 45)\n"
		"branch  0 taken 0% (fallthrough)\n"
		"branch  1 taken 100%\n"
		"    #####:    7:        printf(\"Failure\");\n"
		"call    0 never executed\n"
		"        -:    8:    else\n"
		"        1:    9:        printf(\"Success\");\n"
		"call    0 returned 100%\n"
		"        1:   10:}\n";

	gcovh::parser<std::istringstream> p(s);

	gcovh::coverage_data src = p.parse();

	TEST(src.line_coverage(), 87.5);
	TEST(src.lines_executed(), 7);
	TEST(src.lines_total(), 8);

	gcovh::generate_coverage_report(src, "test3.out.html");

	return 0;
}

int main (int argc, char *argv[]) {
	if (test1() || test2() || test3()) {
		return -1;
	}
	return 0;
}