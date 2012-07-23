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

	TEST((int)lines.size(), 6);
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

	gcovh::write(src, "test1.out.html");

	return 0;
}

int test2(void) {
	gcovh::parser p;

	p.parse("        -:    0:Graph:foo.gcno");
	p.parse("        -:    0:Data:foo.gcda");
	p.parse("      100:    1:int foo (int n) {");
	p.parse("      100:    2:    if (n == 0)");
	p.parse("    #####:    3:        std::cout << \"foo\" std::endl;");
	p.parse("      100:    4:}");

	gcovh::parsed_source src = p.result();

	TEST(src.coverage(), 75.0);
	TEST(src.executed_lines(), 3);
	TEST(src.total_lines(), 4);
	TEST(src.graph(), std::string("foo.gcno"));
	TEST(src.data(), std::string("foo.gcda"));
	
	gcovh::parsed_lines lines = src.all();

	TEST((int)lines.size(), 4);
	TEST(lines[0].executable(), true);
	TEST(lines[1].executable(), true);
	TEST(lines[2].executable(), true);
	TEST(lines[3].executable(), true);

	TEST(lines[0].exec_count(), std::string("100"));
	TEST(lines[1].exec_count(), std::string("100"));
	TEST(lines[2].exec_count(), std::string("#####"));
	TEST(lines[3].exec_count(), std::string("100"));

	gcovh::write(src, "test2.out.html");

	return 0;
}

int test3(void) {
	gcovh::parser p;

	p.parse("function main called 1 returned 100% blocks executed 86%");
	p.parse("        1:    1:int main(void) {");
	p.parse("        -:    2:    int i, total;");
	p.parse("        1:    3:    total = 0;");
	p.parse("       11:    4:    for (i = 0; i < 10; i++)");
	p.parse("branch  0 taken 91%");
	p.parse("branch  1 taken 9% (fallthrough)");
	p.parse("       10:    5:        total += i;");
	p.parse("        1:    6:    if (total != 45)");
	p.parse("branch  0 taken 0% (fallthrough)");
	p.parse("branch  1 taken 100%");
	p.parse("    #####:    7:        printf(\"Failure\");");
	p.parse("call    0 never executed");
	p.parse("        -:    8:    else");
	p.parse("        1:    9:        printf(\"Success\");");
	p.parse("call    0 returned 100%");
	p.parse("        1:   10:}");

	gcovh::parsed_source src = p.result();

	TEST(src.coverage(), 87.5);
	TEST(src.executed_lines(), 7);
	TEST(src.total_lines(), 8);

	gcovh::write(src, "test3.out.html");

	return 0;
}

int main (int argc, char *argv[]) {
	if (test1() || test2() || test3()) {
		return -1;
	}
	return 0;
}