package main

import (
	"fmt"
	"math"
	"time"

	"github.com/soramimi/jstream/jstream-go"
)

const json = `{
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
}`

type Social struct {
	Type string
	URL  string
}

type Change struct {
	Field string
	Foo   jstream.Variant
	Bar   string
}

type History struct {
	Action    string
	Timestamp string
	Changes   []Change
}

type DataItem struct {
	User struct {
		ID      float64
		Name    string
		Roles   []jstream.Variant
		Active  bool
		Profile struct {
			Bio   string
			Links struct {
				Homepage jstream.Variant
				Social   []Social
			}
		}
	}
	History []History
}

type ParsedData struct {
	Meta struct {
		ID        string
		Timestamp string
		Flags     []jstream.Variant
	}
	Config struct {
		Version  string
		Features struct {
			Experimental struct {
				Enabled    bool
				Parameters struct {
					Alpha float64
					Gamma struct {
						Value                      string
						Enabled                    bool
						Notes                      []string
						UnexpectedDeepNestingLevel float64
					}
				}
			}
			Deprecated []jstream.Variant
		}
	}
	Data []DataItem
	Misc struct {
		EmptyObj jstream.Variant
		EmptyArr jstream.Variant
		Types    struct {
			Int     float64
			Float   float64
			String  string
			Boolean bool
			Null    jstream.Variant
			Array   []float64
			Object  jstream.Variant
		}
	}
}

func perform() {
	validation := true

	var parsed ParsedData

	reader := jstream.NewReader(json)
	for reader.Next() {
		if reader.Match("{meta{id") {
			parsed.Meta.ID = reader.StringValue()
		} else if reader.Match("{meta{timestamp") {
			parsed.Meta.Timestamp = reader.StringValue()
		} else if reader.Match("{meta{flags[*") && reader.IsValue() {
			parsed.Meta.Flags = append(parsed.Meta.Flags, reader.GetVariant())
		} else if reader.Match("{config{version") {
			parsed.Config.Version = reader.StringValue()
		} else if reader.Match("{config{features{experimental{enabled") {
			parsed.Config.Features.Experimental.Enabled = reader.BooleanValue()
		} else if reader.Match("{config{features{experimental{parameters{alpha") {
			parsed.Config.Features.Experimental.Parameters.Alpha = reader.Number()
		} else if reader.Match("{config{features{experimental{parameters{gamma{value") {
			parsed.Config.Features.Experimental.Parameters.Gamma.Value = reader.StringValue()
		} else if reader.Match("{config{features{experimental{parameters{gamma{enabled") {
			parsed.Config.Features.Experimental.Parameters.Gamma.Enabled = reader.BooleanValue()
		} else if reader.Match("{config{features{experimental{parameters{gamma{notes[**") {
			if reader.IsConstant() {
				parsed.Config.Features.Experimental.Parameters.Gamma.Notes = append(
					parsed.Config.Features.Experimental.Parameters.Gamma.Notes,
					reader.StringValue(),
				)
			} else if reader.IsStartObject() {
				reader.Nest(func() {
					if reader.Match("{config{features{experimental{parameters{gamma{notes[{unexpected{values[{nesting{level") {
						parsed.Config.Features.Experimental.Parameters.Gamma.UnexpectedDeepNestingLevel = reader.Number()
					}
				})
			}
		} else if reader.Match("{config{features{deprecated[*") && reader.IsValue() {
			parsed.Config.Features.Deprecated = append(parsed.Config.Features.Deprecated, reader.GetVariant())
		} else if reader.Match("{data[*") {
			var data DataItem
			reader.Nest(func() {
				if reader.Match("{data[{user{id") {
					data.User.ID = reader.Number()
				} else if reader.Match("{data[{user{name") {
					data.User.Name = reader.StringValue()
				} else if reader.Match("{data[{user{roles[*") {
					reader.Nest(func() {
						if reader.IsConstant() {
							data.User.Roles = append(data.User.Roles, reader.GetVariant())
						} else if reader.IsStartObject() {
							reader.Nest(func() {
								if reader.Match("{data[{user{roles[*{type") || reader.Match("{data[{user{roles[*{name") {
									v := jstream.NewObject()
									v.AsObject().Set(reader.Key(), jstream.NewString(reader.StringValue()))
									data.User.Roles = append(data.User.Roles, v)
								}
							})
						}
					})
				} else if reader.Match("{data[{user{active") {
					data.User.Active = reader.BooleanValue()
				} else if reader.MatchStartObject("{data[{history[*") {
					var his History
					reader.Nest(func() {
						if reader.Match("{data[{history[{action") {
							his.Action = reader.StringValue()
						} else if reader.Match("{data[{history[{timestamp") {
							his.Timestamp = reader.StringValue()
						} else if reader.Match("{data[{history[{changes[*") {
							var chg Change
							reader.Nest(func() {
								if reader.Match("{data[{history[{changes[{field") {
									chg.Field = reader.StringValue()
								} else if reader.Match("{data[{history[{changes[{foo") {
									chg.Foo = reader.GetVariant()
								} else if reader.Match("{data[{history[{changes[{bar") {
									chg.Bar = reader.StringValue()
								}
							})
							his.Changes = append(his.Changes, chg)
						}
					})
					data.History = append(data.History, his)
				} else if reader.Match("{data[{user{profile{bio") {
					data.User.Profile.Bio = reader.StringValue()
				} else if reader.Match("{data[{user{profile{links{homepage") {
					data.User.Profile.Links.Homepage = reader.GetVariant()
				} else if reader.Match("{data[{user{profile{links{social[*") {
					var soc Social
					reader.Nest(func() {
						if reader.Match("{data[{user{profile{links{social[{type") {
							soc.Type = reader.StringValue()
						} else if reader.Match("{data[{user{profile{links{social[{url") {
							soc.URL = reader.StringValue()
						}
					})
					data.User.Profile.Links.Social = append(data.User.Profile.Links.Social, soc)
				}
			})
			parsed.Data = append(parsed.Data, data)
		} else if reader.Match("{misc{emptyObj") {
			parsed.Misc.EmptyObj = jstream.NewObject()
		} else if reader.Match("{misc{emptyArr") {
			parsed.Misc.EmptyArr = jstream.NewArray()
		} else if reader.Match("{misc{types{int") {
			parsed.Misc.Types.Int = reader.Number()
		} else if reader.Match("{misc{types{float") {
			parsed.Misc.Types.Float = reader.Number()
		} else if reader.Match("{misc{types{string") {
			parsed.Misc.Types.String = reader.StringValue()
		} else if reader.Match("{misc{types{boolean") {
			parsed.Misc.Types.Boolean = reader.BooleanValue()
		} else if reader.Match("{misc{types{null") {
			parsed.Misc.Types.Null = reader.GetVariant()
		} else if reader.Match("{misc{types{array[*") {
			reader.Nest(func() {
				if reader.IsValue() {
					parsed.Misc.Types.Array = append(parsed.Misc.Types.Array, reader.Number())
				}
			})
		} else if reader.Match("{misc{types{object{nested{again{why") {
			parsed.Misc.Types.Object = reader.GetVariant()
		}
	}

	if validation {
		validate(&parsed)
	}
}

func validate(parsed *ParsedData) {
	assertEq := func(name string, a, b interface{}) {
		if a != b {
			panic(fmt.Sprintf("%s: expected %v, got %v", name, b, a))
		}
	}
	expectNear := func(name string, a, b, tol float64) {
		if math.Abs(a-b) > tol {
			panic(fmt.Sprintf("%s: expected %v, got %v", name, b, a))
		}
	}

	assertEq("meta.id", parsed.Meta.ID, "9f3c1a2b-d3e7-43d2-b3a5-92024ad58e3a")
	assertEq("meta.timestamp", parsed.Meta.Timestamp, "2025-04-22T12:34:56Z")
	if len(parsed.Meta.Flags) != 7 {
		panic(fmt.Sprintf("meta.flags length: expected 7, got %d", len(parsed.Meta.Flags)))
	}
	assertEq("meta.flags[0]", parsed.Meta.Flags[0].Boolean(), true)
	assertEq("meta.flags[1]", parsed.Meta.Flags[1].Boolean(), false)
	assertEq("meta.flags[2] is null", parsed.Meta.Flags[2].IsNull(), true)
	assertEq("meta.flags[3]", parsed.Meta.Flags[3].String(), "yes")
	assertEq("meta.flags[4]", parsed.Meta.Flags[4].Number(), 0.0)
	assertEq("meta.flags[5]", parsed.Meta.Flags[5].Number(), 1.0)
	assertEq("meta.flags[6]", parsed.Meta.Flags[6].String(), "0.0")

	assertEq("config.version", parsed.Config.Version, "v1.0.0-beta+exp.sha.5114f85")
	assertEq("config.features.experimental.enabled", parsed.Config.Features.Experimental.Enabled, true)
	expectNear("config.features.experimental.parameters.alpha", parsed.Config.Features.Experimental.Parameters.Alpha, 0.001, 1e-10)
	assertEq("config.features.experimental.parameters.gamma.value", parsed.Config.Features.Experimental.Parameters.Gamma.Value, "∞")
	assertEq("config.features.experimental.parameters.gamma.enabled", parsed.Config.Features.Experimental.Parameters.Gamma.Enabled, false)
	if len(parsed.Config.Features.Experimental.Parameters.Gamma.Notes) != 2 {
		panic(fmt.Sprintf("gamma.notes length: expected 2, got %d", len(parsed.Config.Features.Experimental.Parameters.Gamma.Notes)))
	}
	assertEq("gamma.notes[0]", parsed.Config.Features.Experimental.Parameters.Gamma.Notes[0], "Supports Unicode ✓")
	assertEq("gamma.notes[1]", parsed.Config.Features.Experimental.Parameters.Gamma.Notes[1], "Handles emoji 😊")
	assertEq("gamma.unexpected_deep_nesting_level", parsed.Config.Features.Experimental.Parameters.Gamma.UnexpectedDeepNestingLevel, 6.0)
	if len(parsed.Config.Features.Deprecated) != 3 {
		panic(fmt.Sprintf("deprecated length: expected 3, got %d", len(parsed.Config.Features.Deprecated)))
	}
	assertEq("deprecated[0]", parsed.Config.Features.Deprecated[0].String(), "featureX")
	assertEq("deprecated[1]", parsed.Config.Features.Deprecated[1].String(), "featureY")
	assertEq("deprecated[2] is null", parsed.Config.Features.Deprecated[2].IsNull(), true)

	if len(parsed.Data) != 2 {
		panic(fmt.Sprintf("data length: expected 2, got %d", len(parsed.Data)))
	}
	assertEq("data[0].user.id", parsed.Data[0].User.ID, 1.0)
	assertEq("data[0].user.name", parsed.Data[0].User.Name, "Alice")
	if len(parsed.Data[0].User.Roles) != 4 {
		panic(fmt.Sprintf("data[0].user.roles length: expected 4, got %d", len(parsed.Data[0].User.Roles)))
	}
	assertEq("data[0].user.roles[0]", parsed.Data[0].User.Roles[0].String(), "admin")
	assertEq("data[0].user.roles[1]", parsed.Data[0].User.Roles[1].String(), "user")
	assertEq("data[0].user.roles[2].type", parsed.Data[0].User.Roles[2].AsObject().Get("type").String(), "custom")
	assertEq("data[0].user.roles[3].name", parsed.Data[0].User.Roles[3].AsObject().Get("name").String(), "α-β")
	assertEq("data[0].user.active", parsed.Data[0].User.Active, true)
	if len(parsed.Data[0].History) != 2 {
		panic(fmt.Sprintf("data[0].history length: expected 2, got %d", len(parsed.Data[0].History)))
	}
	assertEq("data[0].history[0].action", parsed.Data[0].History[0].Action, "login")
	assertEq("data[0].history[1].action", parsed.Data[0].History[1].Action, "update")

	assertEq("data[1].user.profile.bio", parsed.Data[1].User.Profile.Bio, "👨‍💻 Coder. \"Escape\\Sequence\" tester.")
	assertEq("data[1].user.profile.links.homepage is null", parsed.Data[1].User.Profile.Links.Homepage.IsNull(), true)
	if len(parsed.Data[1].User.Profile.Links.Social) != 2 {
		panic(fmt.Sprintf("social length: expected 2, got %d", len(parsed.Data[1].User.Profile.Links.Social)))
	}
	assertEq("social[0].type", parsed.Data[1].User.Profile.Links.Social[0].Type, "twitter")
	assertEq("social[0].url", parsed.Data[1].User.Profile.Links.Social[0].URL, "https://twitter.com/bob")
	assertEq("social[1].type", parsed.Data[1].User.Profile.Links.Social[1].Type, "matrix")
	assertEq("social[1].url", parsed.Data[1].User.Profile.Links.Social[1].URL, "matrix:r/room:server")

	assertEq("misc.emptyObj is object", parsed.Misc.EmptyObj.IsObject(), true)
	assertEq("misc.emptyObj length", parsed.Misc.EmptyObj.AsObject().Len(), 0)
	assertEq("misc.emptyArr is array", parsed.Misc.EmptyArr.IsArray(), true)
	assertEq("misc.emptyArr length", parsed.Misc.EmptyArr.AsArray().Len(), 0)
	assertEq("misc.types.int", parsed.Misc.Types.Int, 42.0)
	expectNear("misc.types.float", parsed.Misc.Types.Float, 3.14159, 1e-10)
	assertEq("misc.types.string", parsed.Misc.Types.String, "test")
	assertEq("misc.types.boolean", parsed.Misc.Types.Boolean, false)
	assertEq("misc.types.null is null", parsed.Misc.Types.Null.IsNull(), true)
	if len(parsed.Misc.Types.Array) != 3 {
		panic(fmt.Sprintf("misc.types.array length: expected 3, got %d", len(parsed.Misc.Types.Array)))
	}
	assertEq("misc.types.array[0]", parsed.Misc.Types.Array[0], 1.0)
	assertEq("misc.types.array[1]", parsed.Misc.Types.Array[1], 2.0)
	assertEq("misc.types.array[2]", parsed.Misc.Types.Array[2], 3.0)
	assertEq("misc.types.object", parsed.Misc.Types.Object.String(), "not")
}

func main() {
	iterations := 100000
	fmt.Printf("Parsing JSON %d times...\n", iterations)

	start := time.Now()
	for i := 0; i < iterations; i++ {
		perform()
	}

	fmt.Printf("Parsed JSON in %d ms\n", time.Since(start).Milliseconds())
}
