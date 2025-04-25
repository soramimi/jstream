
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
			"history": [
				{"action": "login", "timestamp": "2025-01-01T00:00:00Z"},
				{"action": "update", "changes": [{"field": "email", "foo": null, "bar": "a@example.com"}]}
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
					std::string bio;
					struct Links {
						Variant homepage;
						struct Social {
							std::string type;
							std::string url;
						};
						std::vector<Social> social;
					} links;
				} profile;
			} user;
			struct History {
				std::string action;
				std::string timestamp;
				struct Change {
					std::string field;
					std::string foo;
					std::string bar;
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
				std::string object_nested_again_why;
			} types;
		} misc;
	} parsed;

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
		} else if (reader.match("{data[*")) {
			if (reader.is_start_object()) {
				ParsedData::Data data;
				reader.nest();
				do {
					if (reader.match("{data[{user{id")) {
						data.user.id = reader.number();
					} else if (reader.match("{data[{user{name")) {
						data.user.name = reader.string();
					} else if (reader.match("{data[{user{roles[*")) {
						if (reader.isvalue()) {
							data.user.roles += var(reader);
						} else if (reader.is_start_object()) {
							Variant v;
							reader.nest();
							do {
								if (reader.match("{data[{user{roles[{type")) {
									obj(v)[reader.key()] = reader.string();
								} else if (reader.match("{data[{user{roles[{name")) {
									obj(v)[reader.key()] = reader.string();
								}
							} while (reader.next());
							data.user.roles.push_back(v);
						}
					} else if (reader.match("{data[{user{active")) {
						data.user.active = reader.istrue();
					} else if (reader.match("{data[{history[*")) {
						if (reader.is_start_object()) {
							ParsedData::Data::History his;
							reader.nest();
							do {
								if (reader.match("{data[{history[{action")) {
									his.action = reader.string();
								} else if (reader.match("{data[{history[{timestamp")) {
									his.timestamp = reader.string();
								} else if (reader.match("{data[{history[{changes[*")) {
									if (reader.is_start_object()) {
										ParsedData::Data::History::Change chg;
										reader.nest();
										do {
											if (reader.match("{data[{history[{changes[{field")) {
												chg.field = reader.string();
											} else if (reader.match("{data[{history[{changes[{foo")) {
												chg.foo = reader.string();
											} else if (reader.match("{data[{history[{changes[{bar")) {
												chg.bar = reader.string();
											}
										} while (reader.next());
										his.changes.push_back(chg);
									}
								}
							} while (reader.next());
							data.history.push_back(his);
						}
					} else if (reader.match("{data[{user{profile{bio")) {
						data.user.profile.bio = reader.string();
					} else if (reader.match("{data[{user{profile{links{homepage")) {
						data.user.profile.links.homepage = var(reader);
					} else if (reader.match("{data[{user{profile{links{social[*")) {
						ParsedData::Data::User::Profile::Links::Social so;
						reader.nest();
						do {
							if (reader.match("{data[{user{profile{links{social[{type")) {
								so.type = reader.string();
							} else if (reader.match("{data[{user{profile{links{social[{url")) {
								so.url = reader.string();
							}
						} while (reader.next());
						data.user.profile.links.social.push_back(so);
					}
				} while (reader.next());
				parsed.data.push_back(data);
			}
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
			if (reader.isvalue()) {
				parsed.misc.types.array.push_back(reader.number());
			}
		} else if (reader.match("{misc{types{object{nested{again{why")) {
			parsed.misc.types.object_nested_again_why = reader.string();
		}
	}

	if (reader.has_error()) {
		for (auto const &e : reader.errors()) {
			std::cerr << e.what() << std::endl;
		}
	}

	EXPECT_EQ(get<std::string>(parsed.meta.id), "9f3c1a2b-d3e7-43d2-b3a5-92024ad58e3a");
	EXPECT_EQ(get<std::string>(parsed.meta.timestamp), "2025-04-22T12:34:56Z");
	assert(parsed.meta.flags.size() == 7);
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
	assert(parsed.data.size() == 2);
	EXPECT_EQ(get<double>(parsed.data[0].user.id), 1);
	EXPECT_EQ(get<std::string>(parsed.data[0].user.name), "Alice");
	EXPECT_EQ(get<bool>(parsed.data[0].user.active), true);
	assert(parsed.data[0].user.roles.size() == 3);
	EXPECT_EQ(parsed.data[0].user.roles.get<std::string>(0), "admin");
	EXPECT_EQ(parsed.data[0].user.roles.get<std::string>(1), "user");
	EXPECT_EQ(obj(parsed.data[0].user.roles[2]).get<std::string>("type"), "custom");
	EXPECT_EQ(obj(parsed.data[0].user.roles[2]).get<std::string>("name"), "α-β");
	EXPECT_EQ(get<double>(parsed.data[1].user.id), 2);
	EXPECT_EQ(get<std::string>(parsed.data[1].user.name), "Bob");
	EXPECT_EQ(get<bool>(parsed.data[1].user.active), false);
	EXPECT_EQ(parsed.data[1].user.profile.bio, "👨‍💻 Coder. \"Escape\\Sequence\" tester.");
	EXPECT_TRUE(is_null(parsed.data[1].user.profile.links.homepage));
	EXPECT_EQ(parsed.data[1].user.profile.links.social[0].type, "twitter");
	EXPECT_EQ(parsed.data[1].user.profile.links.social[0].url, "https://twitter.com/bob");
	EXPECT_EQ(parsed.data[1].user.profile.links.social[1].type, "matrix");
	EXPECT_EQ(parsed.data[1].user.profile.links.social[1].url, "matrix:r/room:server");
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

