#ifndef TEST_H
#define TEST_H

#include <string>
#include <vector>
#include "jstream.h"

void test_all(bool print_result);

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

#endif // TEST_H
