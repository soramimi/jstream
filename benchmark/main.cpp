
#include "../include/jstream.h"
#include "ElapsedTimer.h"

using namespace jstream;

void perform()
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
				bool active;
				struct Profile {
					Variant bio;
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
				Variant object;
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
			ParsedData::Data data;
			reader.nest();
			do {
				if (reader.match("{data[{user{id")) {
					data.user.id = reader.number();
				} else if (reader.match("{data[{user{name")) {
					data.user.name = reader.string();
				} else if (reader.match("{data[{user{roles[*")) {
					reader.nest();
					do {
						if (reader.isvalue()) {
							data.user.roles.push_back(var(reader));
						} else if (reader.is_start_object()) {
							reader.nest();
							do {
								if (reader.match("{data[{user{roles[*{type") || reader.match("{data[{user{roles[*{name")) {
									Variant v;
									obj(v)[reader.key()] = reader.string();
									data.user.roles.push_back(v);
								}
							} while (reader.next());
						}
					} while (reader.next());
				} else if (reader.match("{data[{user{active")) {
					data.user.active = reader.istrue();
				} else if (reader.match_start_object("{data[{history[*")) {
					ParsedData::Data::History his;
					reader.nest();
					do {
						if (reader.match("{data[{history[{action")) {
							his.action = reader.string();
						} else if (reader.match("{data[{history[{timestamp")) {
							his.timestamp = reader.string();
						} else if (reader.match("{data[{history[{changes[*")) {
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
					} while (reader.next());
					data.history.push_back(his);
				} else if (reader.match("{data[{user{profile{bio")) {
					std::string s = reader.string();
					data.user.profile.bio = reader.string();
				} else if (reader.match("{data[{user{profile{links{homepage")) {
					data.user.profile.links.homepage = var(reader);
				} else if (reader.match("{data[{user{profile{links{social[*")) {
					ParsedData::Data::User::Profile::Links::Social soc;
					reader.nest();
					do {
						if (reader.match("{data[{user{profile{links{social[{type")) {
							soc.type = reader.string();
						} else if (reader.match("{data[{user{profile{links{social[{url")) {
							soc.url = reader.string();
						}
					} while (reader.next());
					data.user.profile.links.social.push_back(soc);
				}
			} while (reader.next());
			parsed.data.push_back(data);
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
			parsed.misc.types.boolean = reader.istrue();
		} else if (reader.match("{misc{types{null")) {
			parsed.misc.types.null = var(reader);
		} else if (reader.match("{misc{types{array[*")) {
			reader.nest();
			do {
				if (reader.isvalue()) {
					arr(parsed.misc.types.array).push_back(reader.number());
				}
			} while (reader.next());
		} else if (reader.match("{misc{types{object{nested{again{why")) {
			parsed.misc.types.object = var(reader);
		}
	}
}

int main()
{
	int n = 100000;
	printf("Parsing JSON %d times...\n", n);

	ElapsedTimer timer;
	timer.start();

	for (int i = 0; i < n; i++) {
		perform();
	}

	printf("Parsed JSON in %llu ms\n", timer.elapsed());

	return 0;

}
