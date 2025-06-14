using System;
using System.Collections.Generic;
using Xunit;

namespace JStream.Tests;

public class ExtremeTests {
	public class ModelContextProtocol {
		public string Method { get; set; } = string.Empty;

		public class ParamsData {
			public string ProtocolVersion { get; set; } = string.Empty;

			public class CapabilitiesData {
				public class RootsData {
					public bool ListChanged { get; set; }
				}
				public RootsData Roots { get; set; } = new();

				public class PromptsData {
					public bool ListChanged { get; set; }
				}
				public PromptsData Prompts { get; set; } = new();

				public class ResourcesData {
					public bool Subscribe { get; set; }
					public bool ListChanged { get; set; }
				}
				public ResourcesData Resources { get; set; } = new();

				public class ToolsData {
					public bool ListChanged { get; set; }
				}
				public ToolsData Tools { get; set; } = new();
			}
			public CapabilitiesData Capabilities { get; set; } = new();

			public class ClientInfoData {
				public string Name { get; set; } = string.Empty;
				public string Version { get; set; } = string.Empty;
			}
			public ClientInfoData ClientInfo { get; set; } = new();

			public string Name { get; set; } = string.Empty;

			public class Argument {
				public string Name { get; set; } = string.Empty;
				public string Value { get; set; } = string.Empty;
			}
			public List<Argument> Arguments { get; set; } = new();
		}
		public ParamsData Params { get; set; } = new();

		public class ResultData {
			public string ProtocolVersion { get; set; } = string.Empty;

			public class CapabilitiesData {
				public class PromptsData {
					public bool ListChanged { get; set; }
				}
				public PromptsData Prompts { get; set; } = new();

				public class ResourcesData {
					public bool Subscribe { get; set; }
					public bool ListChanged { get; set; }
				}
				public ResourcesData Resources { get; set; } = new();

				public class ToolsData {
					public bool ListChanged { get; set; }
				}
				public ToolsData Tools { get; set; } = new();
			}
			public CapabilitiesData Capabilities { get; set; } = new();

			public class ServerInfoData {
				public string Name { get; set; } = string.Empty;
				public string Version { get; set; } = string.Empty;
			}
			public ServerInfoData ServerInfo { get; set; } = new();

			public class Tool {
				public string Name { get; set; } = string.Empty;
				public string Description { get; set; } = string.Empty;

				public class InputSchemaData {
					public class Property {
						public string Name { get; set; } = string.Empty;
						public string Title { get; set; } = string.Empty;
						public string Type { get; set; } = string.Empty;
					}
					public List<Property> Properties { get; set; } = new();
					public List<string> Required { get; set; } = new();
					public string Title { get; set; } = string.Empty;
					public string Type { get; set; } = string.Empty;
				}
				public InputSchemaData InputSchema { get; set; } = new();
			}
			public List<Tool> Tools { get; set; } = new();
		}
		public ResultData Result { get; set; } = new();

		public string JsonRpc { get; set; } = string.Empty;
		public string Id { get; set; } = string.Empty;
	}

	[Fact]
	public void Extreme1_ComplexJsonParsing_Test()
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

		// Use simple variables instead of complex structures
		string metaId = string.Empty;
		string metaTimestamp = string.Empty;
		var metaFlags = new JArray();

		string configVersion = string.Empty;
		bool experimentalEnabled = false;
		double parametersAlpha = 0.0;
		string gammaValue = string.Empty;
		bool gammaEnabled = false;
		var gammaNotes = new JArray();
		double unexpectedDeepNestingLevel = 0.0;
		var deprecatedFeatures = new JArray();

		var userData = new List<(double id, string name, bool active, string bio, JArray roles)>();

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
			} else if (reader.Match("{meta{flags[*") && reader.IsValue) {
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
				if (reader.IsValue) {
					gammaNotes.Add(new Variant(reader.StringValue));
				} else if (reader.IsStartObject) {
					reader.Nest();
					do {
						if (reader.Match("{config{features{experimental{parameters{gamma{notes[{unexpected{values[{nesting{level")) {
							unexpectedDeepNestingLevel = reader.Number;
						}
					} while (reader.Next());
				}
			} else if (reader.Match("{config{features{deprecated[*") && reader.IsValue) {
				deprecatedFeatures.Add(reader.GetVariant());
			} else if (reader.Match("{data[*")) {
				double userId = 0;
				string userName = string.Empty;
				bool userActive = false;
				string userBio = string.Empty;
				var userRoles = new JArray();

				reader.Nest();
				do {
					if (reader.Match("{data[{user{id")) {
						userId = reader.Number;
					} else if (reader.Match("{data[{user{name")) {
						userName = reader.StringValue;
					} else if (reader.Match("{data[{user{active")) {
						userActive = reader.IsTrue;
					} else if (reader.Match("{data[{user{profile{bio")) {
						userBio = reader.StringValue;
					} else if (reader.Match("{data[{user{roles[*") && reader.IsValue) {
						userRoles.Add(reader.GetVariant());
					} else if (reader.MatchStartObject("{data[{user{roles[*")) {
						// Handle role objects like {"type": "custom", "name": "α-β"}
						var roleObject = new JObject();
						reader.Nest();
						do {
							if (reader.Match("{data[{user{roles[{type")) {
								roleObject["type"] = new Variant(reader.StringValue);
							} else if (reader.Match("{data[{user{roles[{name")) {
								roleObject["name"] = new Variant(reader.StringValue);
							}
						} while (reader.Next());
						userRoles.Add(new Variant(roleObject));
					}
				} while (reader.Next());

				userData.Add((userId, userName, userActive, userBio, userRoles));
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

		if (reader.HasError) {
			foreach (var error in reader.Errors) {
				Console.WriteLine(error);
			}
		}

		// Assertions
		Assert.Equal("9f3c1a2b-d3e7-43d2-b3a5-92024ad58e3a", metaId);
		Assert.Equal("2025-04-22T12:34:56Z", metaTimestamp);
		Assert.Equal(7, metaFlags.Count);

		Assert.True(metaFlags[0].Get<bool>());
		Assert.False(metaFlags[1].Get<bool>());
		Assert.True(metaFlags[2].IsNull);
		Assert.Equal("yes", metaFlags[3].Get<string>());
		Assert.Equal(0.0, metaFlags[4].Get<double>());
		Assert.Equal(1.0, metaFlags[5].Get<double>());
		Assert.Equal("0.0", metaFlags[6].Get<string>());

		Assert.Equal("v1.0.0-beta+exp.sha.5114f85", configVersion);
		Assert.True(experimentalEnabled);
		Assert.Equal(0.001, parametersAlpha);
		Assert.Equal("∞", gammaValue);
		Assert.False(gammaEnabled);

		Assert.Equal(2, gammaNotes.Count);
		Assert.Equal("Supports Unicode ✓", gammaNotes[0].Get<string>());
		Assert.Equal("Handles emoji 😊", gammaNotes[1].Get<string>());
		Assert.Equal(6.0, unexpectedDeepNestingLevel);

		Assert.Equal(3, deprecatedFeatures.Count);
		Assert.Equal("featureX", deprecatedFeatures[0].Get<string>());
		Assert.Equal("featureY", deprecatedFeatures[1].Get<string>());
		Assert.True(deprecatedFeatures[2].IsNull);

		Assert.Equal(2, userData.Count);
		Assert.Equal(1.0, userData[0].id);
		Assert.Equal("Alice", userData[0].name);
		Assert.True(userData[0].active);
		Assert.Equal(3, userData[0].roles.Count);
		Assert.Equal("admin", userData[0].roles[0].Get<string>());
		Assert.Equal("user", userData[0].roles[1].Get<string>());

		// Test the complex role object
		var roleObj = userData[0].roles[2].Get<JObject>();
		Assert.Equal("custom", roleObj.Get<string>("type"));

		Assert.Equal(2.0, userData[1].id);
		Assert.Equal("Bob", userData[1].name);
		Assert.False(userData[1].active);
		Assert.Equal("👨‍💻 Coder. \"Escape\\Sequence\" tester.", userData[1].bio);
		Assert.Equal(0, userData[1].roles.Count);

		Assert.Equal(42.0, typesInt);
		Assert.Equal(3.14159, typesFloat, 5);
		Assert.Equal("test", typesString);
		Assert.False(typesBoolean);
		Assert.True(typesNull);

		Assert.Equal(3, typesArray.Count);
		Assert.Equal(1.0, typesArray[0].Get<double>());
		Assert.Equal(2.0, typesArray[1].Get<double>());
		Assert.Equal(3.0, typesArray[2].Get<double>());
		Assert.Equal("not", typesObject);
	}

	private ModelContextProtocol ParseMcp(string input)
	{
		var mcp = new ModelContextProtocol();
		var tool = new ModelContextProtocol.ResultData.Tool();
		var property = new ModelContextProtocol.ResultData.Tool.InputSchemaData.Property();

		var reader = new Reader(input);
		while (reader.Next()) {
			if (reader.Match("{method")) {
				mcp.Method = reader.StringValue;
			} else if (reader.Match("{params{protocolVersion")) {
				mcp.Params.ProtocolVersion = reader.StringValue;
			} else if (reader.Match("{params{**")) {
				if (reader.Match("{params{capabilities{roots{listChanged")) {
					mcp.Params.Capabilities.Roots.ListChanged = reader.IsTrue;
				} else if (reader.Match("{params{clientInfo{name")) {
					mcp.Params.ClientInfo.Name = reader.StringValue;
				} else if (reader.Match("{params{clientInfo{version")) {
					mcp.Params.ClientInfo.Version = reader.StringValue;
				} else if (reader.Match("{params{name")) {
					mcp.Params.Name = reader.StringValue;
				} else if (reader.Match("{params{arguments{*")) {
					if (reader.IsValue) {
						var argument = new ModelContextProtocol.ParamsData.Argument {
							Name = reader.Key,
							Value = reader.StringValue
						};
						mcp.Params.Arguments.Add(argument);
					}
				}
			} else if (reader.Match("{jsonrpc")) {
				mcp.JsonRpc = reader.StringValue;
			} else if (reader.Match("{id")) {
				mcp.Id = reader.StringValue;
			}
		}

		Assert.False(reader.HasError);
		return mcp;
	}

	[Fact]
	public void MCP1_Initialize_Test()
	{
		const string input = """
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
        """;

		var mcp = ParseMcp(input);
		Assert.Equal("initialize", mcp.Method);
		Assert.Equal("2024-11-05", mcp.Params.ProtocolVersion);
		Assert.True(mcp.Params.Capabilities.Roots.ListChanged);
		Assert.Equal("mcp", mcp.Params.ClientInfo.Name);
		Assert.Equal("2.0", mcp.JsonRpc);
		Assert.Equal("0", mcp.Id);
	}

	[Fact]
	public void MCP2_Notifications_Test()
	{
		const string input = """
        {
            "method": "notifications/initialized",
            "jsonrpc": "2.0",
            "params": null
        }
        """;

		var mcp = ParseMcp(input);
		Assert.Equal("notifications/initialized", mcp.Method);
		Assert.Equal("2.0", mcp.JsonRpc);
	}

	[Fact]
	public void MCP3_ToolsCall_Test()
	{
		const string input = """
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
        """;

		var mcp = ParseMcp(input);
		Assert.Equal("tools/call", mcp.Method);
		Assert.Equal("add", mcp.Params.Name);
		Assert.Equal(2, mcp.Params.Arguments.Count);
		Assert.Equal("a", mcp.Params.Arguments[0].Name);
		Assert.Equal("3", mcp.Params.Arguments[0].Value);
		Assert.Equal("b", mcp.Params.Arguments[1].Name);
		Assert.Equal("4", mcp.Params.Arguments[1].Value);
		Assert.Equal("2.0", mcp.JsonRpc);
		Assert.Equal("25", mcp.Id);
	}
}