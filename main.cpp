

#include "json.h"
#include <stdint.h>
#include <string>
#include <vector>

std::string to_stdstr(std::vector<char> const &vec)
{
	if (!vec.empty()) {
		char const *begin = &vec[0];
		char const *end = begin + vec.size();
		return std::string(begin, end);
	}
	return std::string();
}


class JSON {
public:
private:
	static int scan_space(char const *ptr, char const *end)
	{
		int i;
		for (i = 0; ptr + i < end && isspace(ptr[i] & 0xff); i++);
		return i;
	}
	static int parse_symbol(char const *begin, char const *end, std::string *out)
	{
		char const *ptr = begin;
		ptr += scan_space(ptr, end);
		std::vector<char> vec;
		while (ptr < end) {
			if (!isalnum((unsigned char)*ptr)) break;
			vec.push_back(*ptr);
			ptr++;
		}
		if (ptr > begin && !vec.empty()) {
			*out = to_stdstr(vec);
			return ptr - begin;
		}
		out->clear();
		return 0;
	}
	static int parse_number(char const *begin, char const *end, double *out)
	{
		*out = 0;
		char const *ptr = begin;
		ptr += scan_space(ptr, end);
		std::vector<char> vec;
		while (ptr < end) {
			char c = *ptr;
			if (isdigit((unsigned char)c) || c == '.' || c == '+' || c == '-' || c == 'e' || c == 'E') {
				// thru
			} else {
				break;
			}
			vec.push_back(c);
			ptr++;
		}
		vec.push_back(0);
		*out = strtod(vec.data(), nullptr);
		return ptr - begin;
	}
	static int parse_string(char const *begin, char const *end, std::string *out)
	{
		char const *ptr = begin;
		ptr += scan_space(ptr, end);
		if (*ptr == '\"') {
			ptr++;
			std::vector<char> vec;
			while (ptr < end) {
				if (*ptr == '\"') {
					*out = to_stdstr(vec);
					ptr++;
					return ptr - begin;
				} else if (*ptr == '\\') {
					ptr++;
					if (ptr < end) {
						auto push = [&](char c){ vec.push_back(c); ptr++;};
						switch (*ptr) {
						case 'b': push('\b'); break;
						case 'n': push('\n'); break;
						case 'r': push('\r'); break;
						case 'f': push('\f'); break;
						case 't': push('\t'); break;
						case '\\':
						case '\"':
							push(*ptr);
							break;
						case 'u':
							ptr++;
							if (ptr + 3) {
								char tmp[5];
								tmp[0] = ptr[0];
								tmp[1] = ptr[1];
								tmp[2] = ptr[2];
								tmp[3] = ptr[3];
								tmp[4] = 0;
								uint16_t unicode = (uint16_t)strtol(tmp, nullptr, 16);
								if (unicode < (1 << 7)) {
									vec.push_back(unicode & 0x7f);
								} else if (unicode < (1 << 11)) {
									vec.push_back(((unicode << 2) & 0x1f) | 0xc0);
									vec.push_back((unicode & 0x3f) | 0x80);
								} else if (unicode < (1 << 16)) {
									vec.push_back(((unicode << 4) & 0x0f) | 0xe0);
									vec.push_back(((unicode << 2) & 0x3f) | 0x80);
									vec.push_back((unicode & 0x3f) | 0x80);
								} else if (unicode < (1 << 21)) {
									vec.push_back(((unicode << 6) & 0x07) | 0xf0);
									vec.push_back(((unicode << 4) & 0x3f) | 0x80);
									vec.push_back(((unicode << 2) & 0x3f) | 0x80);
									vec.push_back((unicode & 0x3f) | 0x80);
								}
								ptr += 4;
							}
							break;
						default:
							if (*ptr >= '0' && *ptr <= '7') {
								int i;
								int v = 0;
								for (i = 0; i < 3; i++) {
									if (ptr + i < end && ptr[i] >= '0' && ptr[i] <= '7') {
										v = v * 8 + (ptr[i] - '0');
									} else {
										break;
									}
								}
								vec.push_back(v);
								ptr += i;
							} else {
								vec.push_back(*ptr);
								ptr++;
							}
							break;
						}
					}
				} else {
					vec.push_back(*ptr);
					ptr++;
				}
			}
		}
		return 0;
	}
public:
	JSON(char const *begin, char const *end)
	{
		d = {};
		d.begin = begin;
		d.end = end;
		d.ptr = d.begin;
	}
	enum StateType {
		None,
		StartObject,
		EndObject,
		StartArray,
		EndArray,
		Comma,
		Key,
		Null,
		False,
		True,
		String,
		Number,
	};
private:
	struct ParserData {
		char const *begin = nullptr;
		char const *end = nullptr;
		char const *ptr = nullptr;
		std::vector<StateType> state_type_;
		std::string key;
		std::string string;
		double number = 0;
		bool is_array = false;
		std::vector<std::string> depth;
	};
	ParserData d;

	void push_state(StateType s)
	{
		if (state() == Key || state() == Comma || state() == EndObject) {
			d.state_type_.pop_back();
		}
		d.state_type_.push_back(s);

		switch (s) {
		case StartArray:
			d.is_array = true;
			break;
		case StartObject:
		case Key:
			d.is_array = false;
			break;
		}
	}

	bool pop_state()
	{
		bool f = false;
		if (!d.state_type_.empty()) {
			d.state_type_.pop_back();
			size_t i = d.state_type_.size();
			while (i > 0) {
				i--;
				auto s = d.state_type_[i];
				if (s == StartArray) {
					d.is_array = true;
					break;
				}
				if (s == StartObject || s == Key) {
					d.is_array = false;
					break;
				}
			}
			f = true;
			if (state() == Key) {
				d.state_type_.pop_back();
			}
		}
		d.key.clear();
		return f;
	}
public:
	bool next()
	{
		d.ptr += scan_space(d.ptr, d.end);
		if (d.ptr < d.end) {
			if (*d.ptr == '}') {
				d.ptr++;
				d.string.clear();
				std::string key;
				if (!d.depth.empty()) {
					key = d.depth.back();
					auto n = key.size();
					if (n > 0) {
						if (key[n - 1] == '{') {
							key = key.substr(0, n - 1);
						}
					}
					d.depth.pop_back();
				}
				while (1) {
					bool f = (state() == StartObject);
					if (!pop_state()) break;
					if (f) {
						push_state(EndObject);
						d.key = key;
						return true;
					}
				}
			}
			if (*d.ptr == ']') {
				d.ptr++;
				d.string.clear();
				if (!d.depth.empty()) d.depth.pop_back();
				while (1) {
					bool f = (state() == StartArray);
					if (!pop_state()) break;
					if (f) {
						push_state(EndArray);
						return true;
					}
				}
			}
			if (*d.ptr == ',') {
				d.ptr++;
				d.ptr += scan_space(d.ptr, d.end);
				if (is_value()) {
					pop_state();
				}
				push_state(Comma);
			}
			if (*d.ptr == '{') {
				d.ptr++;
				d.depth.push_back(d.key + '{');
				push_state(StartObject);
				return true;
			}
			if (*d.ptr == '[') {
				d.ptr++;
				d.depth.push_back(d.key + '[');
				d.key.clear();
				push_state(StartArray);
				return true;
			}
			if (*d.ptr == '\"') {
				auto n = parse_string(d.ptr, d.end, &d.string);
				if (n > 0) {
					if (is_array()) {
						d.ptr += n;
						push_state(String);
						return true;
					}
					if (state() == Key) {
						d.ptr += n;
						push_state(String);
						return true;
					}
					n += scan_space(d.ptr + n, d.end);
					if (d.ptr[n] == ':') {
						d.ptr += n + 1;
						d.key = d.string;
						push_state(Key);
						return true;
					}
				}
			}
			if (isdigit((unsigned char)*d.ptr) || *d.ptr == '-') {
				auto n = parse_number(d.ptr, d.end, &d.number);
				if (n > 0) {
					d.ptr += n;
					push_state(Number);
					return true;
				}
			}
			if (isalpha((unsigned char)*d.ptr)) {
				auto n = parse_symbol(d.ptr, d.end, &d.string);
				if (n > 0) {
					if (state() == Key || state() == StartArray) {
						d.ptr += n;
						if (d.string == "false") {
							push_state(False);
						} else if (d.string == "true") {
							push_state(True);
						} else {
							push_state(Null);
						}
						return true;
					}
				}
			}
		}

		return false;
	}

	StateType state() const
	{
		return d.state_type_.empty() ? None : d.state_type_.back();
	}

	bool is_value() const
	{
		switch (state()) {
		case StartObject:
		case StartArray:
		case String:
		case Number:
		case Null:
		case False:
		case True:
			return true;
		}
		return false;
	}
	std::string key() const
	{
		return d.key;
	}
	std::string string() const
	{
		return d.string;
	}
	double number() const
	{
		return d.number;
	}
	bool is_array() const
	{
		return d.is_array;
	}
	int depth() const
	{
		return d.depth.size();
	}
	std::string path() const
	{
		std::string path;
		for (std::string const &s : d.depth) {
			path += s;
		}
		return path + d.key;
	}
	bool match(char const *path) const
	{
		for (std::string const &s : d.depth) {
			if (strncmp(path, s.c_str(), s.size()) != 0) return false;
			path += s.size();
			if (*path == '*' && path[1] == 0) {
				return true;
			}
		}
		return path == d.key;
	}
};

int main()
{
	static char const source[] = R"---(
								 {
									 "glossary": {
										 "title": "example glossary",
										 "GlossDiv": {
											 "title": "S",
											 "GlossList": {
												 "GlossEntry": {
													 "ID": "SGML",
													 "SortAs": "SGML",
													 "GlossTerm": "Standard Generalized Markup Language",
													 "Acronym": "SGML",
													 "Abbrev": "ISO 8879:1986",
													 "GlossDef": {
														 "para": "A meta-markup language, used to create markup languages such as DocBook.",
														 "GlossSeeAlso": ["GML", "XML"]
													 },
													 "GlossSee": "markup"
												 }
											 }
										 }
									 }
								 }
								 )---";

	std::string para;

	JSON json(source, source + strlen(source));
	while (json.next()) {
		switch (json.state()) {
		case JSON::StartObject:
			printf("StartObject: \"%s\"\n", json.key().c_str());
			break;
		case JSON::EndObject:
			printf("EndObject: \"%s\"\n", json.key().c_str());
			break;
		case JSON::StartArray:
			printf("StartArray:\n");
			break;
		case JSON::EndArray:
			printf("EndArray:\n");
			break;
		case JSON::String:
			printf("StringValue: \"%s\" = \"%s\"\n", json.key().c_str(), json.string().c_str());
			break;
		case JSON::Number:
			printf("NumberValue: \"%s\" = %f\n", json.key().c_str(), json.number());
			break;
		case JSON::Null:
		case JSON::False:
		case JSON::True:
			printf("Value: \"%s\" = %s\n", json.key().c_str(), json.string().c_str());
			break;
		}
		if (json.match("{glossary{GlossDiv{GlossList{GlossEntry{GlossDef{para")) {
			para = json.string();
		}
	}

	printf("\n");
	printf("para = \"%s\"\n", para.c_str());

	return 0;
}
