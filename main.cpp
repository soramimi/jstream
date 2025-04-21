
#include "jstream.h"
#include "test.h"
#include <stdio.h>
#include <map>
#include <iostream>
#include <assert.h>

// testing model context protocol

struct ModelContextProtocol {
	std::string method;
	struct Params {
		std::string protocolVersion;
		struct Capabilities {
			struct Roots {
				bool listChanged = false;
			} roots;
			struct Prompts {
				bool listChanged = false;
			} prompts;
			struct Resources {
				bool subscribe = false;
				bool listChanged = false;
			} resources;
			struct Tools {
				bool listChanged = false;
			} tools;
		} capabilities;
		struct ClientInfo {
			std::string name;
			std::string version;
		} clientInfo;
		std::string name;
		struct Argument {
			std::string name;
			std::string value;
		};
		std::vector<Argument> arguments;
	} params;
	struct Result {
		std::string protocolVersion;
		struct Capabilities {
			struct Experimental {
			} experimental;
			struct Prompts {
				bool listChanged = false;
			} prompts;
			struct Resources {
				bool subscribe = false;
				bool listChanged = false;
			} resources;
			struct Tools {
				bool listChanged = false;
			} tools;
		} capabilities;
		struct ServerInfo {
			std::string name;
			std::string version;
		} serverInfo;
		struct Tool {
			std::string name;
			std::string description;
			struct InputSchema {
				struct Property {
					std::string name;
					std::string title;
					std::string type;
				};
				std::vector<Property> properties;
				std::vector<std::string> required;
				std::string title;
				std::string type;
			} inputSchema;
		};
		std::vector<Tool> tools;
	} result;
	std::string jsonrpc;
	std::string id;
};

ModelContextProtocol parse_mcp(std::string const &input)
{
	ModelContextProtocol mcp;
	ModelContextProtocol::Result::Tool tool;
	ModelContextProtocol::Result::Tool::InputSchema::Property property;

	jstream::Reader reader(input);
	while (reader.next()) {
		if (reader.match("{method")) {
			mcp.method = reader.string();
		} else if (reader.match("{params{protocolVersion")) {
			mcp.params.protocolVersion = reader.string();
		} else if (reader.match("{params{**")) {
			if (reader.match("{params{capabilities{roots{listChanged")) {
				mcp.params.capabilities.roots.listChanged = reader.istrue();
			} else if (reader.match("{params{clientInfo{name")) {
				mcp.params.clientInfo.name = reader.string();
			} else if (reader.match("{params{clientInfo{version")) {
				mcp.params.clientInfo.version = reader.string();
			} else if (reader.match("{params{name")) {
				mcp.params.name = reader.string();
			} else if (reader.match("{params{arguments{*")) {
				if (reader.isvalue()) {
					ModelContextProtocol::Params::Argument a;
					a.name = reader.key();
					a.value = reader.string();
					mcp.params.arguments.push_back(a);
				}
			}
		} else if (reader.match("{result{**")) {
			if (reader.match("{result{protocolVersion")) {
				mcp.result.protocolVersion = reader.string();
			} else if (reader.match("{result{capabilities{prompts{listChanged")) {
				mcp.result.capabilities.prompts.listChanged = reader.istrue();
			} else if (reader.match("{result{capabilities{resources{subscribe")) {
				mcp.result.capabilities.resources.subscribe = reader.istrue();
			} else if (reader.match("{result{capabilities{resources{listChanged")) {
				mcp.result.capabilities.resources.listChanged = reader.istrue();
			} else if (reader.match("{result{capabilities{tools{listChanged")) {
				mcp.result.capabilities.tools.listChanged = reader.istrue();
			} else if (reader.match("{result{serverInfo{name")) {
				mcp.result.serverInfo.name = reader.string();
			} else if (reader.match("{result{serverInfo{version")) {
				mcp.result.serverInfo.version = reader.string();
			} else if (reader.match("{result{tools[{name")) {
				tool.name = reader.string();
			} else if (reader.match("{result{tools[{description")) {
				tool.description = reader.string();
			} else if (reader.match("{result{tools[{inputSchema{properties{*{title")) {
				property.title = reader.string();
			} else if (reader.match("{result{tools[{inputSchema{properties{*{type")) {
				property.type = reader.string();
			} else if (reader.match("{result{tools[{inputSchema{properties{*")) {
				if (reader.is_start_object()) {
					property.name = reader.key();
				} else if (reader.is_end_object()) {
					tool.inputSchema.properties.push_back(property);
					property = {};
				}
			} else if (reader.match("{result{tools[{inputSchema{required[*")) {
				tool.inputSchema.required.push_back(reader.string());
			} else if (reader.match("{result{tools[{inputSchema{title")) {
				tool.inputSchema.title = reader.string();
			} else if (reader.match("{result{tools[{inputSchema{type")) {
				tool.inputSchema.type = reader.string();
			}
		} else if (reader.match("{jsonrpc")) {
			mcp.jsonrpc = reader.string();
		} else if (reader.match("{id")) {
			mcp.id = reader.string();
		}
		if (reader.is_start_object()) {
			if (reader.path() == "{result{tools[{") {
				tool = {};
			} else if (reader.path() == "{result{tools[{inputSchema{properties{") {
				property = {};
			}
		} else if (reader.is_end_object()) {
			if (reader.path() == "{result{tools[") {
				mcp.result.tools.push_back(tool);
				tool = {};
			}
		}
	}

	assert(!reader.iserror());
	return mcp;
}

void test_mcp1()
{
	std::string input = R"---(
{
	"method": "initialize",
	"params": {
		"protocolVersion": "2024-11-05",
		"capabilities": {
			"roots": {
				"listChanged": true
			}
		},
		"clientInfo": {
			"name": "mcp",
			"version": "0.1.0"
		}
	},
	"jsonrpc": "2.0",
	"id": 0
}
)---";


	ModelContextProtocol mcp = parse_mcp(input);
	assert(mcp.method == "initialize");
	assert(mcp.params.protocolVersion == "2024-11-05");
	assert(mcp.params.capabilities.roots.listChanged == true);
	assert(mcp.params.clientInfo.name == "mcp");
	assert(mcp.params.clientInfo.version == "0.1.0");
	assert(mcp.jsonrpc == "2.0");
	assert(mcp.id == "0");
}

void test_mcp2()
{
	std::string input = R"---(
{
	"method": "notifications/initialized",
	"jsonrpc": "2.0",
	"params": null
}
)---";


	ModelContextProtocol mcp = parse_mcp(input);
	assert(mcp.method == "notifications/initialized");
	assert(mcp.jsonrpc == "2.0");

}

void test_mcp3()
{
	std::string input = R"---(
{
	"method":"tools/call",
	"params":{
		"name":"add",
		"arguments":{
			"a":3,
			"b":4
		}
	},
	"jsonrpc":"2.0",
	"id":25
}
)---";


	ModelContextProtocol mcp = parse_mcp(input);
	assert(mcp.method == "tools/call");
	assert(mcp.params.name == "add");
	assert(mcp.params.arguments.size() == 2);
	assert(mcp.params.arguments[0].name == "a");
	assert(mcp.params.arguments[0].value == "3");
	assert(mcp.params.arguments[1].name == "b");
	assert(mcp.params.arguments[1].value == "4");
	assert(mcp.jsonrpc == "2.0");
	assert(mcp.id == "25");
}

void test_mcp()
{
	test_mcp1();
	test_mcp2();
	test_mcp3();
}

// testing google access token

struct GoogleAccessToken {
	std::string access_token;
	std::string token_type;
	std::string expires_in;
	std::string scope;
	std::string refresh_token;
	std::string id_token;

	std::string stringify() const
	{
		std::string ret;

		jstream::Writer w([&](char const *p, int n){
			ret += std::string{p, (size_t)n};
		});

		w.object({}, [&](){
			w.string("access_token", access_token);
			w.string("expires_in", expires_in);
			w.string("scope", scope);
			w.string("token_type", token_type);
			w.string("id_token", id_token);
		});

		return ret;
	}
};

std::ostream &operator << (std::ostream &out, GoogleAccessToken const &token)
{
	out << token.stringify();
	return out;
}


GoogleAccessToken test_google_access_token()
{
	std::string input = R"---(
{
"access_token": "qwerty123",
"expires_in": 3599,
"scope": "https://www.googleapis.com/auth/userinfo.profile",
"token_type": "Bearer",
"id_token": "abcdefg"
}
)---";

	GoogleAccessToken out;
	jstream::Reader reader(input);
	while (reader.next()) {
		if (reader.match("{access_token")) {
			out.access_token = reader.string();
		} else if (reader.match("{expires_in")) {
			out.expires_in = reader.string();
		} else if (reader.match("{scope")) {
			out.scope = reader.string();
		} else if (reader.match("{token_type")) {
			out.token_type = reader.string();
		} else if (reader.match("{id_token")) {
			out.id_token = reader.string();
		}
	}
	return out;
}

GoogleAccessToken test_google_access_token2()
{
	std::string input = R"---(
{
"access_token": "qwerty123",
"expires_in": 3599,
"scope": "https://www.googleapis.com/auth/userinfo.profile",
"token_type": "Bearer",
"id_token": "abcdefg"
}
)---";

	GoogleAccessToken out;
	jstream::Reader::rule_for_string_t rule;
	jstream::Reader reader(input);
	rule["{access_token"] = &out.access_token;
	rule["{expires_in"] = &out.expires_in;
	rule["{scope"] = &out.scope;
	rule["{token_type"] = &out.token_type;
	rule["{id_token"] = &out.id_token;
	reader.parse(rule);

	return out;
}

void main2()
{
	setlocale(LC_ALL, "fr-FR");
	char const *json = R"---(
{ "num": 123456.789 }
)---";
	jstream::Reader r(json);
	while (r.next()) {
		if (r.match("{num")) {
			printf("%f\n", r.number());
		}
	}

	jstream::Writer w([](char const *p, int n) {
		fwrite(p, 1, n, stdout);
	});
	w.object({}, [&]() {
		w.number("num", 123456.789);
	});
}

int main()
{
#if 1
	test_all(true);
#elif 1
	main2();
#elif 0
	test_mcp();
#else
	auto t = test_google_access_token();
	std::cout << t;
#endif

	return 0;
}
