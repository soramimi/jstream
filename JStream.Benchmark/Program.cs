using JStream;
using System;
using System.Collections.Generic;

namespace JStream.Benchmark;

class Program
{
    static void Main(string[] args)
    {
        int iterations = args.Length > 0 && int.TryParse(args[0], out int n) ? n : 100000;
        
        Console.WriteLine($"Parsing JSON {iterations} times...");

        var timer = new ElapsedTimer();
        timer.Start();

        for (int i = 0; i < iterations; i++)
        {
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

        // Use simple variables instead of complex structures (same as ExtremeTests)
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
        while (reader.Next())
        {
            if (reader.Match("{meta{id"))
            {
                metaId = reader.StringValue;
            }
            else if (reader.Match("{meta{timestamp"))
            {
                metaTimestamp = reader.StringValue;
            }
            else if (reader.Match("{meta{flags[*") && reader.IsValue)
            {
                metaFlags.Add(reader.GetVariant());
            }
            else if (reader.Match("{config{version"))
            {
                configVersion = reader.StringValue;
            }
            else if (reader.Match("{config{features{experimental{enabled"))
            {
                experimentalEnabled = reader.IsTrue;
            }
            else if (reader.Match("{config{features{experimental{parameters{alpha"))
            {
                parametersAlpha = reader.Number;
            }
            else if (reader.Match("{config{features{experimental{parameters{gamma{value"))
            {
                gammaValue = reader.StringValue;
            }
            else if (reader.Match("{config{features{experimental{parameters{gamma{enabled"))
            {
                gammaEnabled = reader.IsTrue;
            }
            else if (reader.Match("{config{features{experimental{parameters{gamma{notes[**"))
            {
                if (reader.IsValue)
                {
                    gammaNotes.Add(new Variant(reader.StringValue));
                }
                else if (reader.IsStartObject)
                {
                    reader.Nest();
                    do
                    {
                        if (reader.Match("{config{features{experimental{parameters{gamma{notes[{unexpected{values[{nesting{level"))
                        {
                            unexpectedDeepNestingLevel = reader.Number;
                        }
                    } while (reader.Next());
                }
            }
            else if (reader.Match("{config{features{deprecated[*") && reader.IsValue)
            {
                deprecatedFeatures.Add(reader.GetVariant());
            }
            else if (reader.Match("{data[*"))
            {
                double userId = 0;
                string userName = string.Empty;
                bool userActive = false;
                string userBio = string.Empty;
                var userRoles = new JArray();
                
                reader.Nest();
                do
                {
                    if (reader.Match("{data[{user{id"))
                    {
                        userId = reader.Number;
                    }
                    else if (reader.Match("{data[{user{name"))
                    {
                        userName = reader.StringValue;
                    }
                    else if (reader.Match("{data[{user{active"))
                    {
                        userActive = reader.IsTrue;
                    }
                    else if (reader.Match("{data[{user{profile{bio"))
                    {
                        userBio = reader.StringValue;
                    }
                    else if (reader.Match("{data[{user{roles[*") && reader.IsValue)
                    {
                        userRoles.Add(reader.GetVariant());
                    }
                    else if (reader.MatchStartObject("{data[{user{roles[*"))
                    {
                        // Handle role objects like {"type": "custom", "name": "α-β"}
                        var roleObject = new JObject();
                        reader.Nest();
                        do
                        {
                            if (reader.Match("{data[{user{roles[{type"))
                            {
                                roleObject["type"] = new Variant(reader.StringValue);
                            }
                            else if (reader.Match("{data[{user{roles[{name"))
                            {
                                roleObject["name"] = new Variant(reader.StringValue);
                            }
                        } while (reader.Next());
                        userRoles.Add(new Variant(roleObject));
                    }
                } while (reader.Next());
                
                userData.Add((userId, userName, userActive, userBio, userRoles));
            }
            else if (reader.Match("{misc{types{int"))
            {
                typesInt = reader.Number;
            }
            else if (reader.Match("{misc{types{float"))
            {
                typesFloat = reader.Number;
            }
            else if (reader.Match("{misc{types{string"))
            {
                typesString = reader.StringValue;
            }
            else if (reader.Match("{misc{types{boolean"))
            {
                typesBoolean = reader.IsTrue;
            }
            else if (reader.Match("{misc{types{null"))
            {
                typesNull = reader.IsNull;
            }
            else if (reader.Match("{misc{types{array[*"))
            {
                reader.Nest();
                do
                {
                    if (reader.IsValue)
                    {
                        typesArray.Add(new Variant(reader.Number));
                    }
                } while (reader.Next());
            }
            else if (reader.Match("{misc{types{object{nested{again{why"))
            {
                typesObject = reader.StringValue;
            }
        }
    }
}
