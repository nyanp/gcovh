#pragma once 

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>
#include <sstream>

namespace gcovh {

std::vector<std::string> split(const std::string& s, char sep) {
	std::vector<std::string> ret;

	for (int i = 0, n; i <= s.length(); i = n + 1) {
		n = s.find_first_of(sep, i);
		if (n == std::string::npos) 
			n = s.length();
		ret.push_back(s.substr(i, n - i));
	}
	return ret;
}

template<typename Iter>
std::string merge(Iter first, Iter last, char sep) {
	std::string ret;
	Iter it = first;

	for (; it != last; ++it) {
		if (it != first) ret += sep;
		ret += (*it);
	}
	return ret;
}

template <typename Targ, typename Src>
Targ lexical_cast (const Src& arg) {
	std::stringstream ss;
	Targ ret;

	if (!(ss << arg && ss >> ret && ss.eof())) 
		throw std::bad_cast();

	return ret;
}

class parsed_line {
public:
	parsed_line(int line_number, const std::string& content)
		: line_number_(line_number), is_executable_(false), content_(content), execution_count_("-") {}
	parsed_line(int line_number, const std::string& content, int execution_count)
		: line_number_(line_number), is_executable_(true), content_(content), execution_count_(lexical_cast<std::string>(execution_count)) {}

	const std::string& exec_count(void) const {
		return execution_count_;
	}

	bool executable(void) const {
		return is_executable_;
	}

	bool executed(void) const {
		return execution_count_.c_str() != "0";
	}

	int number(void) const {
		return line_number_;
	}

	const std::string& content(void) const {
		return content_;
	}

private:
	int         line_number_;
	bool        is_executable_;
	std::string content_;
	std::string execution_count_;
};

typedef std::vector<parsed_line> parsed_lines;

class parsed_source {
public:
	parsed_source(void) 
		: runs(0), programs(0), lines_executed(0), lines_total(0), coverage(0.0) {}

	void add (const parsed_line& line) {
		lines.push_back(line);
		if (line.executable()) {
			if (line.executed()) 
				lines_executed++;
			lines_total++;
			update_coverage();
		}
	}

	std::string   source_file;
	std::string   graph_file;
	std::string   data_file;
	int           runs;
	int           programs;
	int           lines_executed;
	int           lines_total;
	double        coverage;
	parsed_lines  lines;

private:
	void update_coverage(void) {
		if (lines_total == 0)
			return;
		coverage = 100.0 * lines_executed / lines_total;
	}
};

class parser {
public:
	parser(void) : is_header(true) {

	}

	void parse(const std::string& s) {
		if (is_header_line(s)) {
			assert(is_header == true);
			parse_header(s);
		} else {
			is_header = false;
			parse_content(s);
		}
	}

	parsed_source result(void) const {
		return src;
	}

private:
	typedef std::vector<std::string> line_t;

	bool is_header_line(const std::string& s) const {
		return s.find(" 0:") != std::string::npos;
	}

	bool is_executable_line(const line_t& line) const {
		return line[0].find('-') == std::string::npos;
	}

	int get_execution_count(const line_t& line) const {
		return lexical_cast<int>(line[0]);
	}

	int get_line_number(const line_t& line) const {
		return lexical_cast<int>(line[1]);
	}

	std::string get_source_line_text(const line_t& line) const {
		return merge(line.begin() + 2, line.end(), ':');
	}

	void parse_header(const std::string& s) {
		// -:0:<tag>:<value>
		line_t line = split(s, ':');

		if (line.size() < 4) {
			throw std::domain_error("invalid format");
		}

		std::string tag = line[2];
		std::string value = line[3];

		if (tag == "Source") {
			src.source_file = value;
		} else if (tag == "Graph") {
			src.graph_file  = value;
		} else if (tag == "Data") {
			src.data_file   = value;
		} else if (tag == "Runs") {
			src.runs        = lexical_cast<int>(value);
		} else if (tag == "Programs") {
			src.programs   = lexical_cast<int>(value);
		} else {
			
		}
	}

	void parse_content(const std::string& s) {
		// <execution_count>:<line_number>:<source line text>
		line_t line = split(s, ':');

		if (line.size() < 3) {
			throw std::domain_error("invalid format");
		}

		if (is_executable_line(line)) {
			src.add(parsed_line(get_line_number(line), get_source_line_text(line), get_execution_count(line)));
		} else {
			src.add(parsed_line(get_line_number(line), get_source_line_text(line)));
		}
		
	}

	bool is_header;
	parsed_source src;
};

class writer {
public:
	writer(const std::string& path) {
		fp = fopen(path.c_str(), "w");
	}

	~writer(void) {
		fclose(fp);
	}

	bool fail (void) const {
		return fp == 0;
	}

	void write_header(const parsed_source& src) {
		write_header_tag(src.source_file);
		write_summary(src);
	}

	void write_content(const parsed_lines& content) {
		fprintf(fp, 
			"<h1>Source</h1>"
			"  <pre class=\"source\">");
		for (parsed_lines::const_iterator it = content.begin(), end = content.end(); it != end; ++it) {
			write_oneline(*it);
		}
		fprintf(fp, "</pre>");
	}

	void write_footer(const parsed_source& src) {

	}

private:
	void write_header_tag(const std::string& source_file) {
		fprintf(fp,
			"<head>\n"
			"  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\n"
			"  <title>gcov - %s</title>\n"
			"  <link rel=\"stylesheet\" type=\"text/css\" href=\"gcov.css\">\n"
			"</head>\n",
			source_file.c_str());
	}

	void write_summary(const parsed_source& src) {
		fprintf(fp,
			"<h1>Summary</h1>\n"
			"  <p>Lines executed:%d of %d (%.2f&#37;)</p>\n",
			src.lines_executed,
			src.lines_total,
			src.coverage);
	}

	void write_oneline(const parsed_line& line) {
		fprintf(fp,
			"    <span class=\"lineNum\">%5d</span>%s%6s:%s%s\n",
			line.number(),
			line.executable() ? (line.executed() ? "<span class = \"lineCov\">" : "<span class = \"lineNoCov\">" ) : "",
			line.exec_count().c_str(),
			line.content().c_str(),
			line.executable() ? "</span>" : ""			
			);
	}

	FILE *fp;
};

parsed_source parse (const char* path) {
	std::ifstream ifs(path);
	std::string input;
	parser p;

	if (ifs.fail()) {
		throw std::invalid_argument("failed to open");
	}

	while (std::getline(ifs, input)) {
		p.parse(input);
	}
	return p.result();
}

void write(const parsed_source& src, const std::string& path) {
	writer w(path);

	if (w.fail()) {
		throw std::invalid_argument("failed to open");
	}

	w.write_header(src);
	w.write_content(src.lines);
	w.write_footer(src);
}

}
