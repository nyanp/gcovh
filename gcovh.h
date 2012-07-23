#pragma once 

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cassert>
#include <sstream>
#include <stdexcept>
#include <typeinfo>

namespace gcovh {

namespace detail {

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

std::string trim_begin(const std::string& str) {
	return str.substr(str.find_first_not_of(' '));
}

template <typename Targ, typename Src>
Targ lexical_cast (const Src& arg) {
	std::stringstream ss;
	Targ ret;

	if (!(ss << arg && ss >> ret && ss.eof())) 
		throw std::bad_cast();

	return ret;
}

} // namespace detail

//-------------------------------------------------
// struct of parsed-file

class parsed_line {
public:
	parsed_line(int line_number, const std::string& content)
		: line_number_(line_number), is_executable_(false), content_(content), execution_count_("-") {}
	parsed_line(int line_number, const std::string& content, const std::string& execution_count)
		: line_number_(line_number), is_executable_(true), content_(content), execution_count_(execution_count) {}

	const std::string& exec_count(void) const {
		return execution_count_;
	}

	bool executable(void) const {
		return is_executable_;
	}

	bool executed(void) const {
		return execution_count_[0] != '#';
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
		: source_file("N/A"), graph_file("N/A"), data_file("N/A"),
		runs(0), programs(0), lines_executed(0), lines_total(0), line_coverage(0.0) {}

	void add (const parsed_line& line) {
		contents.push_back(line);
		if (line.executable()) {
			if (line.executed()) 
				lines_executed++;
			lines_total++;
			update_coverage();
		}
	}

	void set_header(const std::string& tag, const std::string& value) {
		if (tag == "Source") 
			source_file = value;
		else if (tag == "Graph")
			graph_file  = value;
		else if (tag == "Data")
			data_file   = value;
		else if (tag == "Runs")
			runs        = detail::lexical_cast<int>(value);
		else if (tag == "Programs")
			programs    = detail::lexical_cast<int>(value);
		else
			return; // thru
	}

	std::string source(void) const {
		return source_file;
	}

	std::string graph(void) const {
		return graph_file;
	}

	std::string data(void) const {
		return data_file;
	}

	int executed_lines(void) const {
		return lines_executed;
	}

	int total_lines(void) const {
		return lines_total;
	}

	double coverage(void) const {
		return line_coverage;
	}

	const parsed_lines& all(void) const {
		return contents;
	}

private:
	std::string   source_file;
	std::string   graph_file;
	std::string   data_file;
	int           runs;
	int           programs;
	int           lines_executed;
	int           lines_total;
	double        line_coverage;
	parsed_lines  contents;

	void update_coverage(void) {
		if (lines_total == 0)
			return;
		line_coverage = 100.0 * lines_executed / lines_total;
	}
};

//-------------------------------------------------
// parser/writer

class parser {
public:
	parser(void) : is_header(true), is_error(false) {}

	bool parse(const std::string& s) {
		if (is_header_line(s)) {
			if (is_header == false)
				is_error = true;
			parse_header(s);
		} else {
			is_header = false;
			parse_content(s);
		}
		return is_error;
	}

	parsed_source result(void) const {
		if (is_error) {
			throw std::domain_error("invalid result"); // throw if getting result from error state
		}
		return src;
	}

	bool error(void) const {
		return is_error;
	}

private:
	typedef std::vector<std::string> line_t;

	bool is_header_line(const std::string& s) const {
		return s.find(" 0:") != std::string::npos;
	}

	bool is_executable_line(const line_t& line) const {
		return line[0].find('-') == std::string::npos;
	}

	std::string get_execution_count(const line_t& line) const {
		return detail::trim_begin(line[0]);
	}

	int get_line_number(const line_t& line) const {
		return detail::lexical_cast<int>(line[1]);
	}

	std::string get_source_line_text(const line_t& line) const {
		return detail::merge(line.begin() + 2, line.end(), ':');
	}

	void parse_header(const std::string& s) {
		// -:0:<tag>:<value>
		line_t line = detail::split(s, ':');

		if (line.size() < 4) {
			return;
		}

		std::string tag = line[2];
		std::string value = line[3];

		src.set_header(tag, value);
	}

	void parse_content(const std::string& s) {
		// <execution_count>:<line_number>:<source line text>
		line_t line = detail::split(s, ':');

		if (line.size() < 3) {
			return; //thru
		}

		if (is_executable_line(line)) {
			src.add(parsed_line(get_line_number(line), get_source_line_text(line), get_execution_count(line)));
		} else {
			src.add(parsed_line(get_line_number(line), get_source_line_text(line)));
		}
	}

	bool is_header;
	bool is_error;
	parsed_source src;
};

class detail_writer {
public:
	detail_writer(const parsed_source& src, const std::string& path) : src(src) {
		fp = fopen(path.c_str(), "w");
	}

	~detail_writer(void) {
		fclose(fp);
	}

	bool fail (void) const {
		return fp == 0;
	}

	void write_header(void) {
		write_header_tag(src.source());
		write_summary(src);
	}

	void write_content(void) {
		fprintf(fp, 
			"<h1>Source</h1>"
			"  <pre class=\"source\">");
		for (parsed_lines::const_iterator it = src.all().begin(), end = src.all().end(); it != end; ++it) {
			write_oneline(*it);
		}
		fprintf(fp, "</pre>");
	}

	void write_footer(void) {

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
			src.executed_lines(),
			src.total_lines(),
			src.coverage());
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

	parsed_source src;
	FILE *fp;
};

class summary_writer {
public:
	typedef std::map<std::string, parsed_source> sources_t;

	summary_writer(const sources_t& src, const std::string& path) : src(src) {
		fp = fopen(path.c_str(), "w");
	}

	~summary_writer(void) {
		fclose(fp);
	}

	void write_header(void) {
		// not implemented
	}

	void write_content(void) {

	}

	void write_footer(void) {

	}

private:
	sources_t src;
	FILE *fp;
};

//-------------------------------------------------
// 

/**
 * parse a content of .gcov file
 *
 * @param path filename
 */
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

/**
 * generage html report file from parsed_source
 *
 * @param src  report source
 * @param path output filename
 */
void write(const parsed_source& src, const std::string& path) {
	detail_writer w(src, path);

	if (w.fail()) {
		throw std::invalid_argument("failed to open");
	}

	w.write_header();
	w.write_content();
	w.write_footer();
}

} // namespace gcovh
