using JStream;
using System;
using System.Collections.Generic;

namespace JStream.Benchmark;

class Program {
	private class Change
	{
		public string Field = string.Empty;
		public Variant Foo = Variant.Null;
		public string Bar = string.Empty;
	}

	private class History
	{
		public string Action = string.Empty;
		public string Timestamp = string.Empty;
		public List<Change> Changes = new();
	}

	private class Social
	{
		public string Type = string.Empty;
		public string URL = string.Empty;
	}

	private class UserData
	{
		public double Id;
		public string Name = string.Empty;
		public bool Active;
		public string Bio = string.Empty;
		public JArray Roles = new();
		public List<History> History = new();
		public List<Social> Social = new();
	}

	static void Main(string[] args)
	{
		int iterations = args.Length > 0 && int.TryParse(args[0], out int n) ? n : 100000;

		Console.WriteLine($"Parsing JSON {iterations} times...");

		var timer = new ElapsedTimer();
		timer.Start();

		for (int i = 0; i < iterations; i++) {
			Perform();
		}

		Console.WriteLine($"Parsed JSON in {timer.Elapsed()} ms");
	}

	static void Perform()
	{
		const string json = """
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
        """;

		string metaId = string.Empty;
		string metaTimestamp = string.Empty;
		var metaFlags = new JArray();

		string configVersion = string.Empty;
		bool experimentalEnabled = false;
		double parametersAlpha = 0.0;
		string gammaValue = string.Empty;
		bool gammaEnabled = false;
		var gammaNotes = new List<string>();
		double unexpectedDeepNestingLevel = 0.0;
		var deprecatedFeatures = new JArray();

		var userData = new List<UserData>();

		var emptyObj = new JObject();
		var emptyArr = new JArray();
		double typesInt = 0.0;
		double typesFloat = 0.0;
		string typesString = string.Empty;
		bool typesBoolean = false;
		bool typesNull = false;
		var typesArray = new JArray();
		string typesObject = string.Empty;

		var reader = new Reader(json);
		while (reader.Next()) {
			if (reader.Match("{meta{id")) {
				metaId = reader.StringValue;
			} else if (reader.Match("{meta{timestamp")) {
				metaTimestamp = reader.StringValue;
			} else if (reader.Match("{meta{flags[*") && reader.IsConstant) {
				metaFlags.Add(reader.GetVariant());
			} else if (reader.Match("{config{version")) {
				configVersion = reader.StringValue;
			} else if (reader.Match("{config{features{experimental{enabled")) {
				experimentalEnabled = reader.IsTrue;
			} else if (reader.Match("{config{features{experimental{parameters{alpha")) {
				parametersAlpha = reader.Number;
			} else if (reader.Match("{config{features{experimental{parameters{gamma{value")) {
				gammaValue = reader.StringValue;
			} else if (reader.Match("{config{features{experimental{parameters{gamma{enabled")) {
				gammaEnabled = reader.IsTrue;
			} else if (reader.Match("{config{features{experimental{parameters{gamma{notes[**")) {
				if (reader.IsConstant) {
					gammaNotes.Add(reader.StringValue);
				} else if (reader.IsStartObject) {
					reader.Nest();
					do {
						if (reader.Match("{config{features{experimental{parameters{gamma{notes[{unexpected{values[{nesting{level")) {
							unexpectedDeepNestingLevel = reader.Number;
						}
					} while (reader.Next());
				}
			} else if (reader.Match("{config{features{deprecated[*") && reader.IsConstant) {
				deprecatedFeatures.Add(reader.GetVariant());
			} else if (reader.Match("{data[*")) {
				var user = new UserData();

				reader.Nest();
				do {
					if (reader.Match("{data[{user{id")) {
						user.Id = reader.Number;
					} else if (reader.Match("{data[{user{name")) {
						user.Name = reader.StringValue;
					} else if (reader.Match("{data[{user{active")) {
						user.Active = reader.IsTrue;
					} else if (reader.Match("{data[{user{profile{bio")) {
						user.Bio = reader.StringValue;
					} else if (reader.Match("{data[{user{roles[*") && reader.IsConstant) {
						user.Roles.Add(reader.GetVariant());
				} else if (reader.MatchStartObject("{data[{user{roles[*")) {
					// Handle role objects like {"type": "custom", "name": "α-β"}
					reader.Nest();
					do {
						if (reader.Match("{data[{user{roles[{type") || reader.Match("{data[{user{roles[{name")) {
							var roleObject = new JObject();
							roleObject[reader.Key] = new Variant(reader.StringValue);
							user.Roles.Add(new Variant(roleObject));
						}
					} while (reader.Next());
					} else if (reader.MatchStartObject("{data[{history[*")) {
						var his = new History();
						reader.Nest();
						do {
							if (reader.Match("{data[{history[{action")) {
								his.Action = reader.StringValue;
							} else if (reader.Match("{data[{history[{timestamp")) {
								his.Timestamp = reader.StringValue;
							} else if (reader.Match("{data[{history[{changes[*")) {
								var chg = new Change();
								reader.Nest();
								do {
									if (reader.Match("{data[{history[{changes[{field")) {
										chg.Field = reader.StringValue;
									} else if (reader.Match("{data[{history[{changes[{foo")) {
										chg.Foo = reader.GetVariant();
									} else if (reader.Match("{data[{history[{changes[{bar")) {
										chg.Bar = reader.StringValue;
									}
								} while (reader.Next());
								his.Changes.Add(chg);
							}
						} while (reader.Next());
						user.History.Add(his);
					} else if (reader.Match("{data[{user{profile{links{social[*")) {
						var soc = new Social();
						reader.Nest();
						do {
							if (reader.Match("{data[{user{profile{links{social[{type")) {
								soc.Type = reader.StringValue;
							} else if (reader.Match("{data[{user{profile{links{social[{url")) {
								soc.URL = reader.StringValue;
							}
						} while (reader.Next());
						user.Social.Add(soc);
					}
				} while (reader.Next());

				userData.Add(user);
			} else if (reader.Match("{misc{types{int")) {
				typesInt = reader.Number;
			} else if (reader.Match("{misc{types{float")) {
				typesFloat = reader.Number;
			} else if (reader.Match("{misc{types{string")) {
				typesString = reader.StringValue;
			} else if (reader.Match("{misc{types{boolean")) {
				typesBoolean = reader.IsTrue;
			} else if (reader.Match("{misc{types{null")) {
				typesNull = reader.IsNull;
			} else if (reader.Match("{misc{types{array[*")) {
				reader.Nest();
				do {
					if (reader.IsValue) {
						typesArray.Add(new Variant(reader.Number));
					}
				} while (reader.Next());
			} else if (reader.Match("{misc{types{object{nested{again{why")) {
				typesObject = reader.StringValue;
			}
		}

		Validate(
			metaId, metaTimestamp, metaFlags,
			configVersion, experimentalEnabled, parametersAlpha, gammaValue, gammaEnabled, gammaNotes, unexpectedDeepNestingLevel, deprecatedFeatures,
			userData,
			emptyObj, emptyArr, typesInt, typesFloat, typesString, typesBoolean, typesNull, typesArray, typesObject
		);
	}

	static void Validate(
		string metaId, string metaTimestamp, JArray metaFlags,
		string configVersion, bool experimentalEnabled, double parametersAlpha, string gammaValue, bool gammaEnabled, List<string> gammaNotes, double unexpectedDeepNestingLevel, JArray deprecatedFeatures,
		List<UserData> userData,
		JObject emptyObj, JArray emptyArr, double typesInt, double typesFloat, string typesString, bool typesBoolean, bool typesNull, JArray typesArray, string typesObject
	)
	{
		AssertEq("meta.id", metaId, "9f3c1a2b-d3e7-43d2-b3a5-92024ad58e3a");
		AssertEq("meta.timestamp", metaTimestamp, "2025-04-22T12:34:56Z");
		AssertEq("meta.flags.count", metaFlags.Count, 7);
		AssertEq("meta.flags[0]", metaFlags[0].Get<bool>(), true);
		AssertEq("meta.flags[1]", metaFlags[1].Get<bool>(), false);
		AssertEq("meta.flags[2].IsNull", metaFlags[2].IsNull, true);
		AssertEq("meta.flags[3]", metaFlags[3].Get<string>(), "yes");
		AssertEq("meta.flags[4]", metaFlags[4].Get<double>(), 0.0);
		AssertEq("meta.flags[5]", metaFlags[5].Get<double>(), 1.0);
		AssertEq("meta.flags[6]", metaFlags[6].Get<string>(), "0.0");

		AssertEq("config.version", configVersion, "v1.0.0-beta+exp.sha.5114f85");
		AssertEq("config.features.experimental.enabled", experimentalEnabled, true);
		ExpectNear("config.features.experimental.parameters.alpha", parametersAlpha, 0.001, 1e-10);
		AssertEq("config.features.experimental.parameters.gamma.value", gammaValue, "∞");
		AssertEq("config.features.experimental.parameters.gamma.enabled", gammaEnabled, false);
		AssertEq("gamma.notes.count", gammaNotes.Count, 2);
		AssertEq("gamma.notes[0]", gammaNotes[0], "Supports Unicode ✓");
		AssertEq("gamma.notes[1]", gammaNotes[1], "Handles emoji 😊");
		AssertEq("gamma.unexpected_deep_nesting_level", unexpectedDeepNestingLevel, 6.0);
		AssertEq("deprecatedFeatures.count", deprecatedFeatures.Count, 3);
		AssertEq("deprecatedFeatures[0]", deprecatedFeatures[0].Get<string>(), "featureX");
		AssertEq("deprecatedFeatures[1]", deprecatedFeatures[1].Get<string>(), "featureY");
		AssertEq("deprecatedFeatures[2].IsNull", deprecatedFeatures[2].IsNull, true);

		AssertEq("userData.count", userData.Count, 2);
		AssertEq("userData[0].id", userData[0].Id, 1.0);
		AssertEq("userData[0].name", userData[0].Name, "Alice");
		AssertEq("userData[0].roles.count", userData[0].Roles.Count, 4);
		AssertEq("userData[0].roles[0]", userData[0].Roles[0].Get<string>(), "admin");
		AssertEq("userData[0].roles[1]", userData[0].Roles[1].Get<string>(), "user");
		AssertEq("userData[0].roles[2].type", userData[0].Roles[2].AsObject()["type"].Get<string>(), "custom");
		AssertEq("userData[0].roles[3].name", userData[0].Roles[3].AsObject()["name"].Get<string>(), "α-β");
		AssertEq("userData[0].active", userData[0].Active, true);
		AssertEq("userData[0].history.count", userData[0].History.Count, 2);
		AssertEq("userData[0].history[0].action", userData[0].History[0].Action, "login");
		AssertEq("userData[0].history[1].action", userData[0].History[1].Action, "update");

		AssertEq("userData[1].profile.bio", userData[1].Bio, "👨‍💻 Coder. \"Escape\\Sequence\" tester.");
		AssertEq("userData[1].social.count", userData[1].Social.Count, 2);
		AssertEq("userData[1].social[0].type", userData[1].Social[0].Type, "twitter");
		AssertEq("userData[1].social[0].url", userData[1].Social[0].URL, "https://twitter.com/bob");
		AssertEq("userData[1].social[1].type", userData[1].Social[1].Type, "matrix");
		AssertEq("userData[1].social[1].url", userData[1].Social[1].URL, "matrix:r/room:server");

		AssertEq("misc.emptyObj.count", emptyObj.Count, 0);
		AssertEq("misc.emptyArr.count", emptyArr.Count, 0);
		AssertEq("misc.types.int", typesInt, 42.0);
		ExpectNear("misc.types.float", typesFloat, 3.14159, 1e-10);
		AssertEq("misc.types.string", typesString, "test");
		AssertEq("misc.types.boolean", typesBoolean, false);
		AssertEq("misc.types.null", typesNull, true);
		AssertEq("misc.types.array.count", typesArray.Count, 3);
		AssertEq("misc.types.array[0]", typesArray[0].Get<double>(), 1.0);
		AssertEq("misc.types.array[1]", typesArray[1].Get<double>(), 2.0);
		AssertEq("misc.types.array[2]", typesArray[2].Get<double>(), 3.0);
		AssertEq("misc.types.object", typesObject, "not");
	}

	static void AssertEq<T>(string name, T actual, T expected)
	{
		if (!EqualityComparer<T>.Default.Equals(actual, expected)) {
			Console.WriteLine($"ASSERT_EQ failed: {name} expected {expected}, got {actual}");
			Environment.Exit(1);
		}
	}

	static void ExpectNear(string name, double actual, double expected, double tolerance)
	{
		if (Math.Abs(actual - expected) > tolerance) {
			Console.WriteLine($"EXPECT_NEAR failed: {name} expected {expected}, got {actual}");
			Environment.Exit(1);
		}
	}
}
