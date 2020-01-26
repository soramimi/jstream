#include "test.h"
#include "jstream.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>

std::string trim(std::string const &s)
{
	size_t i = 0;
	size_t j = s.size();
	while (i < j && isspace((unsigned char)s[i])) i++;
	while (i < j && isspace((unsigned char)s[j - 1])) j--;
	return s.substr(i, j - i);
}

bool read_file(char const *path, std::vector<char> *out)
{
	bool ok = false;
	out->clear();
	int fd = open(path, O_RDONLY);
	if (fd != -1) {
		struct stat st;
		if (fstat(fd, &st) == 0) {
			ok = true;
			if (st.st_size > 0) {
				out->resize(st.st_size);
				if (read(fd, out->data(), out->size()) != st.st_size) {
					ok = false;
				}
			}
		}
		close(fd);
	}
	return ok;
}

bool parse_test_case(char const *begin, char const *end, std::vector<std::string> *strings)
{
	strings->clear();
	char const *head = begin;
	char const *ptr = begin;
	bool newline = true;
	while (1) {
		int c = 0;
		if (ptr < end) {
			c = (unsigned char)*ptr;
		}
		if (c == 0) {
			strings->push_back(std::string(head, ptr));
			break;
		}
		if (c == '\r' || c == '\n') {
			newline = true;
			ptr++;
		} else {
			if (c == '-' && newline) {
				if (ptr + 3 < end && ptr[1] == '-' && ptr[2] == '-') {
					c = ptr[3];
					if (c == '\r' || c == '\n') {
						strings->push_back(std::string(head, ptr));
						ptr += 4;
						head = ptr;
					}
				}
			}
			newline = false;
			ptr++;
		}
	}
	return strings->size() == 3;
}

void parse(char const *source, std::string *result1, std::string *result2)
{
	jstream::Parser json(source, source + strlen(source));
	while (json.next()) {
		char tmp[1000];
		tmp[0] = 0;
		switch (json.state()) {
		case jstream::StartObject:
			sprintf(tmp, "StartObject: \"%s\"\n", json.key().c_str());
			break;
		case jstream::EndObject:
			sprintf(tmp, "EndObject: \"%s\"\n", json.key().c_str());
			break;
		case jstream::StartArray:
			sprintf(tmp, "StartArray: \"%s\"\n", json.key().c_str());
			break;
		case jstream::EndArray:
			sprintf(tmp, "EndArray: \"%s\"\n", json.key().c_str());
			break;
		case jstream::String:
			sprintf(tmp, "StringValue: \"%s\" = \"%s\"\n", json.key().c_str(), json.string().c_str());
			break;
		case jstream::Number:
			sprintf(tmp, "NumberValue: \"%s\" = %f\n", json.key().c_str(), json.number());
			break;
		case jstream::Null:
		case jstream::False:
		case jstream::True:
			sprintf(tmp, "SymbolValue: \"%s\" = %s\n", json.key().c_str(), json.string().c_str());
			break;
		}
		*result1 += tmp;
		if (json.isvalue()) {
			std::string s = json.path() + '=';
			if (json.state() == jstream::String) {
				s = s + '\"' + json.string() + '\"';
			} else {
				s = s + json.string();
			}
			s += '\n';
			*result2 += s;
		}
	}
	if (json.state() == jstream::SyntaxError) {
		*result1 = json.string();
	}
}

void get_tests(std::string const &loc, std::vector<std::string> *out)
{
	out->clear();
	DIR *dir = opendir(loc.c_str());
	if (dir) {
		while (dirent *d = readdir(dir)) {
			std::string name;
			name = d->d_name;
			if (d->d_type & DT_DIR) {
				continue;
			}
			out->push_back(name);
		}
		closedir(dir);
	}
}

void test_all()
{
	int pass = 0;
	int fail = 0;
	int total = 0;

	std::string dir = "test/";
	std::vector<std::string> tests;
	get_tests(dir.c_str(), &tests);
	std::sort(tests.begin(), tests.end());

	for (std::string const &name : tests) {
		std::string path = dir + name;
		std::vector<char> vec;
		if (read_file(path.c_str(), &vec) && !vec.empty()) {
			total++;
			std::vector<std::string> strings;
			char const *begin = vec.data();
			char const *end = begin + vec.size();
			parse_test_case(begin, end, &strings);
			std::string input;
			std::string events;
			std::string values;
			if (strings.size() == 3) {
				input = strings[0];
				events = strings[1];
				values = strings[2];
			} else {
				fail++;
				printf("#%d TEST CASE SYNTAX ERROR %s\n", total, name.c_str());
				continue;
			}

			std::string result1;
			std::string result2;
			parse(input.c_str(), &result1, &result2);
			bool ok1 = trim(result1) == trim(events);
			bool ok2 = trim(result2) == trim(values);
			if (ok1 && ok2) {
				printf("#%d PASS %s\n", total, name.c_str());
				pass++;
			} else {
				fail++;
				printf("#%d FAIL %s\n", total, name.c_str());
				puts("--- INPUT ---------");
				puts(trim(input).c_str());
			}
			if (!ok1) {
				puts("--- EXPECTED EVENTS ----------");
				puts(trim(events).c_str());
				puts("--- ACTUAL RESULT ----------");
				puts(trim(result1).c_str());
				puts("---");
			}
			if (!ok2) {
				puts("--- EXPECTED VALUES ----------");
				puts(trim(values).c_str());
				puts("--- ACTUAL RESULT ----------");
				puts(trim(result2).c_str());
				puts("---");
			}
		}
	}

	printf("---\n" "TOTAL: %d\n" " PASS: %d\n" " FAIL: %d\n", total, pass, fail);
}


