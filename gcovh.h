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
#include <algorithm>

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

// foo.bar.c -> foo.bar
std::string get_filebase(const std::string& filename) {
	std::vector<std::string> s = split(filename, '.');
	return merge(s.begin(), s.end() - 1, '.');
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

std::string replace(std::string src, const std::string& targ, const std::string& repl) {
	size_t i = 0;

	if (targ.length() == 0)
		return src;

	while (true) {
		i = src.find(targ, i);
		if (i == std::string::npos) 
			return src;
		src.replace(i, targ.length(), repl);
		i += repl.length();
	}
}

std::string escape_for_html(const std::string& base) {
	std::string escaped = base;
	escaped = replace(escaped, "&", "&amp");
	escaped = replace(escaped, "<", "&lt");
	escaped = replace(escaped, ">", "&gt");
	escaped = replace(escaped, "\"", "&quot");

	return escaped;
}

} // namespace detail

//-------------------------------------------------
// struct of parsed-file

class source_line {
public:
	source_line(int line_number, const std::string& content)
		: line_number_(line_number), is_executable_(false), content_(content), execution_count_("-") {}
	source_line(int line_number, const std::string& content, const std::string& execution_count)
		: line_number_(line_number), is_executable_(true), content_(content), execution_count_(execution_count) {}

	const char* exec_count(void) const {
		return execution_count_.c_str();
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

typedef std::vector<source_line> source_lines;

class coverage_data {
public:
	coverage_data(const std::string& parse_file) 
		: parse_file_(parse_file), source_file_("N/A"), graph_file_("N/A"), data_file_("N/A"),
		runs_(0), programs_(0), lines_executed_(0), lines_total_(0), line_coverage_(0.0) {}

	void add (const source_line& line) {
		contents_.push_back(line);
		if (line.executable()) {
			if (line.executed()) 
				lines_executed_++;
			lines_total_++;
			update_coverage();
		}
	}

	void set_header(const std::string& tag, const std::string& value) {
		if (tag == "Source") 
			source_file_ = value;
		else if (tag == "Graph")
			graph_file_  = value;
		else if (tag == "Data")
			data_file_   = value;
		else if (tag == "Runs")
			runs_        = detail::lexical_cast<int>(value);
		else if (tag == "Programs")
			programs_    = detail::lexical_cast<int>(value);
		else
			return; // thru
	}

	std::string parse_file(void) const {
		return parse_file_;
	}

	std::string source_file(void) const {
		return source_file_;
	}

	std::string graph_file(void) const {
		return graph_file_;
	}

	std::string data_file(void) const {
		return data_file_;
	}

	int lines_executed(void) const {
		return lines_executed_;
	}

	int lines_total(void) const {
		return lines_total_;
	}

	double line_coverage(void) const {
		return line_coverage_;
	}

	const source_lines& all(void) const {
		return contents_;
	}

private:
	std::string   parse_file_;
	std::string   source_file_;
	std::string   graph_file_;
	std::string   data_file_;
	int           runs_;
	int           programs_;
	int           lines_executed_;
	int           lines_total_;
	double        line_coverage_;
	source_lines  contents_;

	void update_coverage(void) {
		if (lines_total_ == 0)
			return;
		line_coverage_ = 100.0 * lines_executed_ / lines_total_;
	}
};

//-------------------------------------------------
// parser/html-generator

template<class InputStream>
class parser {
public:
	// [header] -:0:<tag>:<value>
	// [body]   <execution_count>:<line_number>:<source line text>
	typedef std::vector<std::string> line_t;

	parser(const std::string& parse_file) : data_(parse_file), is_(parse_file.c_str()) {}

	coverage_data parse() {
		line_t line;

		while (get_line(is_, line)) {
			if (is_header_line(line))
				parse_header(line);
			else
				parse_body(line);
		}
			
		return data_;
	}

private:
	bool get_line(InputStream& stream, line_t& line) {
		std::string s;

		if (std::getline(stream, s)) {
			line = detail::split(s, ':');
			return true;
		}
		return false;
	}

	bool is_header_line(const line_t& line) const {
		return line.size() >= 2 && get_line_number(line) == 0;
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

	void parse_header(const line_t& line) {
		if (line.size() < 4)
			return;

		// -:0:<tag>:<value>
		data_.set_header(line[2], line[3]);
	}

	void parse_body(const line_t& line) {
		if (line.size() < 3)
			return;

		if (is_executable_line(line)) {
			data_.add(source_line(get_line_number(line), get_source_line_text(line), get_execution_count(line)));
		} else {
			data_.add(source_line(get_line_number(line), get_source_line_text(line)));
		}
	}

	coverage_data data_;
	InputStream is_;
};

template <class Content>
class html_generator {
public:
	html_generator(const char *filename) {
		fp_ = fopen(filename, "w");
		if (!fp_) 
			throw std::invalid_argument("failed to open file");
	}

	void write (const Content& c) {
		write_common_header(page_title(c));
		write_content(fp_, c); // template method
		write_common_footer();
	}

	~html_generator() {
		fclose(fp_);
	}

private:
	void write_common_header(const std::string& page_title) {
		fprintf(fp_,
			"<html>"
			"<head>\n"
			"  <meta http-equiv=\"Content-Type\" content=\"text/html\">\n"
			"  <title>gcov - %s</title>\n"
			"  <link rel=\"stylesheet\" type=\"text/css\" href=\"gcov.css\">\n"
			"</head>\n"
			"<body>\n"
			"<div id=\"wrapper\">\n",
			page_title.c_str());
	}

	void write_common_footer() {
		fprintf(fp_, 
			"</div>\n"
			"</body>\n"
			"</html>\n");
	}

	virtual std::string page_title(const Content& c) = 0;
	virtual void write_content(FILE *fp, const Content& c) = 0;

	FILE *fp_;
};

class report_generator : public html_generator<coverage_data> {
public:
	report_generator(const std::string& path)
		: html_generator<coverage_data>(path.c_str()) {}

	void write_content(FILE *fp, const coverage_data& src) {
		write_linecoverage_summary(fp, src);
		write_annotated_source(fp, src);
	}

private:
	std::string page_title(const coverage_data& cov) {
		return cov.source_file();
	}

	void write_annotated_source(FILE *fp, const coverage_data& cov) {
		fprintf(fp, 
			"<h2>Source</h2>\n"
			"  <pre class=\"source\">");
		for (source_lines::const_iterator it = cov.all().begin(), end = cov.all().end(); it != end; ++it) 
			write_oneline(fp, *it);

		fprintf(fp, "</pre>");
	}

	void write_linecoverage_summary(FILE *fp, const coverage_data& cov) {
		fprintf(fp,
			"<h2>Summary</h2>\n"
			"  <p>Lines executed:%d of %d (%.2f&#37;)</p>\n",
			cov.lines_executed(),
			cov.lines_total(),
			cov.line_coverage());
	}

	void write_oneline(FILE *fp, const source_line& line) {
		std::string content_escaped = detail::escape_for_html(line.content());
		const char *str = content_escaped.c_str();

		fprintf(fp,
			"    <span class=\"lineNum\">%5d</span>%s%6s:%s%s\n",
			line.number(),
			line.executable() ? (line.executed() ? "<span class = \"lineCov\">" : "<span class = \"lineNoCov\">" ) : "",
			line.exec_count(),
			content_escaped.c_str(),
			line.executable() ? "</span>" : "");
	}
};

class summary_generator : public html_generator<std::vector<coverage_data> >{
public:
	typedef std::vector<coverage_data> coverages_t;

	summary_generator(const std::string& path)
		: html_generator<std::vector<coverage_data> >(path.c_str()) {}

	void write_content(FILE *fp, const coverages_t& coverages) {
		write_coverages_summary(fp, coverages);
	}

private:
	std::string page_title(const coverages_t& coverages) {
		return "summary";
	}

	void write_coverages_summary(FILE *fp, const coverages_t& coverages) {
		fprintf(fp,
			"<table>\n"
			"  <tr>\n"
			"    <th>FileName</th>\n"
			"    <th colspan=3>Line Coverage</th>\n"
			"  </tr>\n");

		for (coverages_t::const_iterator it = coverages.begin(), end = coverages.end(); it != end; ++it) {
			const coverage_data& cov = (*it);
			std::string html_file = cov.source_file() + ".html";

			fprintf(fp,
				"  <tr>\n"
				"    <td><a href=\"%s\">%s</a></td>\n"
				"    <td><div class=\"progress\"><div class=\"bar\" style=\"width:%d&#37;;\"></div></div></td>\n"
				"    <td>%.2f&#37;</td>\n"
				"    <td>%d/%d</td>\n"
				"  </tr>\n",
				html_file.c_str(),
				cov.source_file().c_str(),
				(int)(cov.line_coverage()),
				cov.line_coverage(),
				cov.lines_executed(),
				cov.lines_total());
		}

		fprintf(fp, "</table>");
	}
};

//-------------------------------------------------
// 

// parse a content of .gcov file
coverage_data parse (const char* gcov_file_name) {
	std::ifstream ifs(gcov_file_name);
	parser<std::ifstream> p(gcov_file_name);

	if (ifs.fail()) 
		throw std::invalid_argument(std::string("failed to open file ") + gcov_file_name);

	return p.parse();
}

// parse contents of .gcov files
std::vector<coverage_data> parse (int num, const char *path[]) {
	std::vector<coverage_data> sources;

	for (int i = 0; i < num; i++)
		sources.push_back(parse(path[i]));

	return sources;
}

// generage html report file from coverage_data
void generate_coverage_report(const coverage_data& src, const std::string& path) {
	report_generator w(path);

	w.write(src);
}

// generage html report file from coverage_data
void generate_coverage_report(const coverage_data& coverage) {
	const std::string path = detail::get_filebase(coverage.parse_file()) + ".html";

	generate_coverage_report(coverage, path);
}

// generage html summary file from coverage_data array
void generate_coverage_summary(const std::vector<coverage_data>& coverages, const std::string& path = "index.html") {
	gcovh::summary_generator summary(path);

	summary.write(coverages);
}

} // namespace gcovh
