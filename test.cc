#include "gcovh.h"
#include <iostream>
#include <cmath>

using namespace std;

static int case_id = 0;

template<typename T>
bool equal (T a, T b) {
	return a == b;
}

template<>
bool equal<double> (double a, double b) {
	return std::abs(a - b) < 1E10;
}


template<typename T>
bool check (T result, T expected) {
	cout << "case #" << case_id++ << ":";

	if (equal(expected, result)) 
		cout << "passed" << endl;
	else
		cout << "failed! (expected:" << expected << ", result:" << result << endl; 

	return equal(expected, result);
}

#define TEST(expected, result) { if (!check(expected, result)) return -1; }


int test1(void) {
	gcovh::parser p;

	p.parse("        -:    0:Source:foo.c");
	p.parse("        -:    0:Programs:1");
	p.parse("        -:    1:#include <stdio.h>");
	p.parse("        -:    2:");
	p.parse("        1:    3:int main (void) {");
	p.parse("        1:    4:    printf(\"foo\");");
	p.parse("        1:    5:    return 0;");
	p.parse("        -:    6:}");

	gcovh::parsed_source src = p.result();

	TEST(src.coverage(), 100.0);
	TEST(src.executed_lines(), 3);
	TEST(src.total_lines(), 3);
	TEST(src.source(), std::string("foo.c"));
	
	gcovh::parsed_lines lines = src.all();

	TEST(lines[0].executable(), false);
	TEST(lines[1].executable(), false);
	TEST(lines[2].executable(), true);
	TEST(lines[3].executable(), true);
	TEST(lines[4].executable(), true);
	TEST(lines[5].executable(), false);

	TEST(lines[0].exec_count(), std::string("-"));
	TEST(lines[1].exec_count(), std::string("-"));
	TEST(lines[2].exec_count(), std::string("1"));
	TEST(lines[3].exec_count(), std::string("1"));
	TEST(lines[4].exec_count(), std::string("1"));
	TEST(lines[5].exec_count(), std::string("-"));
	
	return 0;
}

int main (int argc, char *argv[]) {
	int r = test1();

	return r;
}