
#include "../test.h"
#include <gtest/gtest.h>
#include <variant>

//

using namespace jstream;

TEST(Json, Extreme1)
{
	char const *json = R"---(
{
	"meta": {
		"id": "9f3c1a2b-d3e7-43d2-b3a5-92024ad58e3a",
		"timestamp": "2025-04-22T12:34:56Z",
		"flags": [true, false, null, "yes", 0, 1.0, "0.0"]
	},
	"config": {
		"version": "v1.0.0-beta+exp.sha.5114f85",
		"features": {
			"experimental": {
				"enabled": true,
				"parameters": {
					"alpha": 0.001,
					"gamma": {
						"value": "∞",
						"enabled": false,
						"notes": [
							"Supports Unicode ✓",
							"Handles emoji 😊",
							{
								"unexpected": {
									"values": ["deep", {"nesting": {"level": 6}}]
								}
							}
						]
					}
				}
			},
			"deprecated": ["featureX", "featureY", null]
		}
	},
	"data": [
		{
			"user": {
				"id": 1,
				"name": "Alice",
				"roles": ["admin", "user", {"type": "custom", "name": "α-β"}],
				"active": true
			},
			"hoge": 1,
			"history": [
				{"action": "login", "timestamp": "2025-01-01T00:00:00Z"},
				{"action": "update", "changes": [{"field": "email", "old": null, "new": "a@example.com"}]}
			]
		},
		{
			"user": {
				"id": 2,
				"name": "Bob",
				"roles": [],
				"active": false,
				"profile": {
					"bio": "👨‍💻 Coder. \"Escape\\Sequence\" tester.",
					"links": {
						"homepage": null,
						"social": [
							{"type": "twitter", "url": "https://twitter.com/bob"},
							{"type": "matrix", "url": "matrix:r/room:server"}
						]
					}
				}
			}
		}
	],
	"misc": {
		"emptyObj": {},
		"emptyArr": [],
		"types": {
			"int": 42,
			"float": 3.14159,
			"string": "test",
			"boolean": false,
			"null": null,
			"array": [1, 2, 3],
			"object": {"nested": {"again": {"why": "not"}}}
		}
	}
}
)---";

	struct ParsedData {
		struct Meta {
			Variant id;
			Variant timestamp;
			Array flags;
		} meta;
		struct Config {
			Variant version;
			struct Features {
				struct Experimental {
					Variant enabled;
					struct Parameters {
						Variant alpha;
						struct Gamma {
							Variant value;
							Variant enabled;
							Array notes;
							double unexpected_deep_nesting_level;
						} gamma;
					} parameters;
				} experimental;
				struct Deprecated {
					Array features;
				} deprecated;
			} features;
		} config;
		struct Data {
			struct User {
				Variant id;
				Variant name;
				Array roles;
				Variant active;
				struct Profile {
					Variant bio;
					struct Links {
						Variant homepage;
						Array social;
					} links;
				} profile;
			} user;
			struct History {
				std::string action;
				std::string timestamp;
				struct Change {
					std::string field;
					Variant foo;
					Variant bar;
				};
				std::vector<Change> changes;
			};
			std::vector<History> history;
		};
		std::vector<Data> data;
		struct Misc {
			Variant emptyObj;
			Variant emptyArr;
			struct Types {
				Variant int_;
				Variant float_;
				Variant string;
				Variant boolean;
				Variant null;
				Array array;
				Object object;
			} types;
		} misc;
	} parsed;

	ParsedData::Data data;

	jstream::Reader reader(json);
	while (reader.next()) {
		if (reader.match("{meta{id")) {
			parsed.meta.id = reader.string();
		} else if (reader.match("{meta{timestamp")) {
			parsed.meta.timestamp = reader.string();
		} else if (reader.match("{meta{flags[*") && reader.isvalue()) {
			parsed.meta.flags += var(reader);
		} else if (reader.match("{config{version")) {
			parsed.config.version = reader.string();
		} else if (reader.match("{config{features{experimental{enabled")) {
			parsed.config.features.experimental.enabled = reader.istrue();
		} else if (reader.match("{config{features{experimental{parameters{alpha")) {
			parsed.config.features.experimental.parameters.alpha = reader.number();
		} else if (reader.match("{config{features{experimental{parameters{gamma{value")) {
			parsed.config.features.experimental.parameters.gamma.value = reader.string();
		} else if (reader.match("{config{features{experimental{parameters{gamma{enabled")) {
			parsed.config.features.experimental.parameters.gamma.enabled = reader.istrue();
		} else if (reader.match("{config{features{experimental{parameters{gamma{notes[**")) {
			if (reader.isvalue()) {
				parsed.config.features.experimental.parameters.gamma.notes.push_back(reader.string());
			} else if (reader.is_start_object()) {
				reader.nest();
				do {
					if (reader.match("{config{features{experimental{parameters{gamma{notes[{unexpected{values[{nesting{level")) {
						parsed.config.features.experimental.parameters.gamma.unexpected_deep_nesting_level = reader.number();
					}
				} while (reader.next());
			}
		} else if (reader.match("{config{features{deprecated[*") && reader.isvalue()) {
			parsed.config.features.deprecated.features += var(reader);
		} else if (reader.match("{data[{user{id")) {
			data.user.id = reader.number();
		} else if (reader.match("{data[{user{name")) {
			data.user.name = reader.string();
		} else if (reader.match("{data[{user{roles[*") && reader.isvalue()) {
			data.user.roles += var(reader);
		} else if (reader.match("{data[{user{active")) {
			data.user.active = reader.string();
		} else if (reader.match("{data[{hoge")) {
			// printf("@ %d %s %s\n", reader.state(), reader.key().c_str(), reader.string().c_str());
		} else if (reader.match("{data[{history[*")) {
			if (reader.is_start_object()) {
				ParsedData::Data::History his;
				reader.nest();
				do {
					// {"action": "login", "timestamp": "2025-01-01T00:00:00Z"},
					// {"action": "update", "changes": [{"field": "email", "old": null, "new": "a@example.com"}]}
					if (reader.match("{data[{history[{action")) {
						his.action = reader.string();
						// printf("@ %d %s %s\n", reader.state(), reader.key().c_str(), reader.string().c_str());
					}
					ParsedData::Data::History::Change chg;

				} while (reader.next());
			}
		} else if (reader.match("{data[{profile{bio")) {
			data.user.profile.bio = reader.string();
		} else if (reader.match("{data[{profile{links{homepage")) {
			data.user.profile.links.homepage = reader.string();
		} else if (reader.match("{data[{profile{links{social[*")) {
			// todo:
		} else if (reader.match("{misc{emptyObj")) {
			parsed.misc.emptyObj = reader.string();
		} else if (reader.match("{misc{emptyArr")) {
			parsed.misc.emptyArr = reader.string();
		} else if (reader.match("{misc{types{int")) {
			parsed.misc.types.int_ = reader.number();
		} else if (reader.match("{misc{types{float")) {
			parsed.misc.types.float_ = reader.number();
		} else if (reader.match("{misc{types{string")) {
			parsed.misc.types.string = reader.string();
		} else if (reader.match("{misc{types{boolean")) {
			parsed.misc.types.boolean = reader.string();
		} else if (reader.match("{misc{types{null")) {
			parsed.misc.types.null = reader.string();
		} else if (reader.match("{misc{types{array[*")) {
			// todo:
		} else if (reader.match("{misc{types{object{{nested{{again{{why")) {
			// todo:
		}
	}

	if (reader.has_error()) {
		for (auto const &e : reader.errors()) {
			std::cerr << e.what() << std::endl;
		}
	}

	EXPECT_EQ(get<std::string>(parsed.meta.id), "9f3c1a2b-d3e7-43d2-b3a5-92024ad58e3a");
	EXPECT_EQ(get<std::string>(parsed.meta.timestamp), "2025-04-22T12:34:56Z");
	ASSERT_EQ(parsed.meta.flags.size(), 7);
	EXPECT_EQ(get<bool>(parsed.meta.flags[0]), true);
	EXPECT_EQ(get<bool>(parsed.meta.flags[1]), false);
	EXPECT_EQ(get<null_t>(parsed.meta.flags[2]), null);
	EXPECT_EQ(get<std::string>(parsed.meta.flags[3]), "yes");
	EXPECT_EQ(get<double>(parsed.meta.flags[4]), 0);
	EXPECT_EQ(get<double>(parsed.meta.flags[5]), 1.0);
	EXPECT_EQ(get<std::string>(parsed.meta.flags[6]), "0.0");
	EXPECT_EQ(get<std::string>(parsed.config.version), "v1.0.0-beta+exp.sha.5114f85");
	EXPECT_EQ(get<bool>(parsed.config.features.experimental.enabled), true);
	EXPECT_EQ(get<double>(parsed.config.features.experimental.parameters.alpha), 0.001);
	EXPECT_EQ(get<std::string>(parsed.config.features.experimental.parameters.gamma.value), "∞");
	EXPECT_EQ(get<bool>(parsed.config.features.experimental.parameters.gamma.enabled), false);
	EXPECT_EQ(parsed.config.features.experimental.parameters.gamma.notes.size(), 2);
	EXPECT_EQ(parsed.config.features.experimental.parameters.gamma.notes.get<std::string>(0), "Supports Unicode ✓");
	EXPECT_EQ(parsed.config.features.experimental.parameters.gamma.notes.get<std::string>(1), "Handles emoji 😊");
	EXPECT_EQ(parsed.config.features.experimental.parameters.gamma.unexpected_deep_nesting_level, 6);
	EXPECT_EQ(parsed.config.features.deprecated.features.size(), 3);
	EXPECT_EQ(get<std::string>(parsed.config.features.deprecated.features[0]), "featureX");
	EXPECT_EQ(get<std::string>(parsed.config.features.deprecated.features[1]), "featureY");
	EXPECT_EQ(get<null_t>(parsed.config.features.deprecated.features[2]), null);
	// EXPECT_EQ(data.user.id, 1);
	// EXPECT_EQ(js::get<std::string>(data.user.name), "Alice");
	// EXPECT_EQ(data.user.roles.size(), 3);
	// EXPECT_EQ(js::get<std::string>(data.user.roles[0]->value), "admin");
	// EXPECT_EQ(js::get<std::string>(data.user.roles[1]->value), "user");
	// EXPECT_EQ(js::get<std::string>(data.user.roles[2]->value), "α-β");
	// EXPECT_EQ(js::get<Enum>(data.user.active), True);
	// EXPECT_EQ(data.history.size(), 2);
	// EXPECT_EQ(js::get<std::string>(data.history[0]->value), "login");
	// EXPECT_EQ(js::get<std::string>(data.history[1]->value), "update");
	// EXPECT_EQ(js::get<std::string>(parsed.user.profile.bio), "👨‍💻 Coder. \"Escape\\Sequence\" tester.");
}

// testing model context protocol

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

	assert(!reader.has_error());
	return mcp;
}

TEST(Json, MCP1)
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
	EXPECT_EQ(mcp.method, "initialize");
	EXPECT_EQ(mcp.params.protocolVersion, "2024-11-05");
	EXPECT_EQ(mcp.params.capabilities.roots.listChanged, true);
	EXPECT_EQ(mcp.params.clientInfo.name, "mcp");
	EXPECT_EQ(mcp.jsonrpc, "2.0");
	EXPECT_EQ(mcp.id, "0");
}

TEST(Json, MCP2)
{
	std::string input = R"---(
{
	"method": "notifications/initialized",
	"jsonrpc": "2.0",
	"params": null
}
)---";


	ModelContextProtocol mcp = parse_mcp(input);
	EXPECT_EQ(mcp.method, "notifications/initialized");
	EXPECT_EQ(mcp.jsonrpc, "2.0");
}

TEST(Json, MCP3)
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
	EXPECT_EQ(mcp.method, "tools/call");
	EXPECT_EQ(mcp.params.name, "add");
	EXPECT_EQ(mcp.params.arguments.size(), 2);
	EXPECT_EQ(mcp.params.arguments[0].name, "a");
	EXPECT_EQ(mcp.params.arguments[0].value, "3");
	EXPECT_EQ(mcp.params.arguments[1].name, "b");
	EXPECT_EQ(mcp.params.arguments[1].value, "4");
	EXPECT_EQ(mcp.jsonrpc, "2.0");
	EXPECT_EQ(mcp.id, "25");
}

