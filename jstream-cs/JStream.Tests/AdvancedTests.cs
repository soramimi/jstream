using System;
using System.Collections.Generic;
using Xunit;

namespace JStream.Tests;

public class AdvancedTests {
	[Fact]
	public void Reader_ComplexNesting_Test()
	{
		const string json = """
        {
            "name": "John",
            "age": 30,
            "city": "New York",
            "address": {
                "street": "123 Main St",
                "zip": "10001"
            },
            "hobby": [
                "reading",
                "traveling",
                "coding",
                "cooking",
                "gaming"
            ]
        }
        """;

		var reader = new Reader(json);
		string? name = null;
		double age = 0;
		string? city = null;
		var address = new JObject();
		var hobby = new JArray();

		while (reader.Next()) {
			if (reader.Match("{name")) {
				name = reader.StringValue;
			} else if (reader.Match("{age")) {
				age = reader.Number;
			} else if (reader.Match("{city")) {
				city = reader.StringValue;
			} else if (reader.Match("{address{street")) {
				address["street"] = new Variant(reader.StringValue);
			} else if (reader.Match("{address{zip")) {
				address["zip"] = new Variant(reader.StringValue);
			} else if (reader.Match("{hobby[*") && reader.IsString) {
				hobby.Add(new Variant(reader.StringValue));
			}
		}

		Assert.Equal("John", name);
		Assert.Equal(30, age);
		Assert.Equal("New York", city);
		Assert.Equal("123 Main St", address.Get<string>("street"));
		Assert.Equal("10001", address.Get<string>("zip"));
		Assert.Equal(5, hobby.Count);
		Assert.Equal("reading", hobby.Get<string>(0));
		Assert.Equal("traveling", hobby.Get<string>(1));
		Assert.Equal("coding", hobby.Get<string>(2));
		Assert.Equal("cooking", hobby.Get<string>(3));
		Assert.Equal("gaming", hobby.Get<string>(4));
	}

	[Fact]
	public void Reader_FloatingPointPrecision_Test()
	{
		const string json = """
        {
          "floats": {
            "smallest_subnormal": 5e-324,
            "largest_normal": 1.7976931348623157e+308,
            "just_over_one": 1.0000000000000002,
            "just_under_one": 0.9999999999999999,
            "pi_high_prec": 3.14159265358979323846264338327950288419716939937510,
            "e_high_prec": 2.71828182845904523536028747135266249775724709369995,
            "half": 0.5,
            "one_third": 0.3333333333333333,
            "binary_rounding_issue_1": 0.1,
            "binary_rounding_issue_2": 0.2,
            "precision_loss": 9007199254740993,
            "int_boundary": 9007199254740991
          }
        }
        """;

		var parsed = new Dictionary<string, double>();
		var reader = new Reader(json);

		while (reader.Next()) {
			if (reader.Match("{floats{smallest_subnormal"))
				parsed["smallest_subnormal"] = reader.Number;
			else if (reader.Match("{floats{largest_normal"))
				parsed["largest_normal"] = reader.Number;
			else if (reader.Match("{floats{just_over_one"))
				parsed["just_over_one"] = reader.Number;
			else if (reader.Match("{floats{just_under_one"))
				parsed["just_under_one"] = reader.Number;
			else if (reader.Match("{floats{pi_high_prec"))
				parsed["pi_high_prec"] = reader.Number;
			else if (reader.Match("{floats{e_high_prec"))
				parsed["e_high_prec"] = reader.Number;
			else if (reader.Match("{floats{half"))
				parsed["half"] = reader.Number;
			else if (reader.Match("{floats{one_third"))
				parsed["one_third"] = reader.Number;
			else if (reader.Match("{floats{binary_rounding_issue_1"))
				parsed["binary_rounding_issue_1"] = reader.Number;
			else if (reader.Match("{floats{binary_rounding_issue_2"))
				parsed["binary_rounding_issue_2"] = reader.Number;
			else if (reader.Match("{floats{precision_loss"))
				parsed["precision_loss"] = reader.Number;
			else if (reader.Match("{floats{int_boundary"))
				parsed["int_boundary"] = reader.Number;
		}

		double eps = 1e-15;

		Assert.True(Math.Abs(parsed["smallest_subnormal"] - 5e-324) < eps || parsed["smallest_subnormal"] == 5e-324);
		Assert.True(Math.Abs(parsed["largest_normal"] - 1.7976931348623157e+308) < 1e+293);
		Assert.True(Math.Abs(parsed["just_over_one"] - 1.0000000000000002) < eps);
		Assert.True(Math.Abs(parsed["just_under_one"] - 0.9999999999999999) < eps);
		Assert.True(Math.Abs(parsed["pi_high_prec"] - 3.14159265358979323846264338327950288419716939937510) < eps);
		Assert.True(Math.Abs(parsed["e_high_prec"] - 2.71828182845904523536028747135266249775724709369995) < eps);
		Assert.Equal(0.5, parsed["half"]);
		Assert.True(Math.Abs(parsed["one_third"] - 0.3333333333333333) < eps);
		Assert.True(Math.Abs(parsed["binary_rounding_issue_1"] - 0.1) < eps);
		Assert.True(Math.Abs(parsed["binary_rounding_issue_2"] - 0.2) < eps);
		Assert.Equal(9007199254740993, parsed["precision_loss"]);
		Assert.Equal(9007199254740991, parsed["int_boundary"]);
	}

	[Fact]
	public void Reader_ArrayOfObjects_Test()
	{
		const string json = """
        [
          {
            "name": "John Smith",
            "age": 34,
            "address": "Brooklyn, New York"
          },
          {
            "name": "Emily Johnson",
            "age": 28,
            "address": "Downtown, Los Angeles"
          },
          {
            "name": "Michael Brown",
            "age": 41,
            "address": "Capitol Hill, Washington D.C."
          }
        ]
        """;

		var persons = new List<(string name, int age, string address)>();
		var reader = new Reader(json);

		while (reader.Next()) {
			if (reader.IsStartArray) {
				reader.Nest();
				do {
					if (reader.IsStartObject) {
						reader.Nest();
						string? name = null;
						int age = 0;
						string? address = null;

						do {
							if (reader.Match("[{name"))
								name = reader.StringValue;
							else if (reader.Match("[{age"))
								age = (int)reader.Number;
							else if (reader.Match("[{address"))
								address = reader.StringValue;
						} while (reader.Next());

						if (name != null && address != null)
							persons.Add((name, age, address));
					}
				} while (reader.Next());
			}
		}

		Assert.Equal(3, persons.Count);
		Assert.Equal("John Smith", persons[0].name);
		Assert.Equal(34, persons[0].age);
		Assert.Equal("Brooklyn, New York", persons[0].address);
		Assert.Equal("Emily Johnson", persons[1].name);
		Assert.Equal(28, persons[1].age);
		Assert.Equal("Downtown, Los Angeles", persons[1].address);
		Assert.Equal("Michael Brown", persons[2].name);
		Assert.Equal(41, persons[2].age);
		Assert.Equal("Capitol Hill, Washington D.C.", persons[2].address);
	}

	[Fact]
	public void Writer_ArrayRoundTrip_Test()
	{
		var originalPersons = new List<(string name, int age, string address)>
		{
			("John Smith", 34, "Brooklyn, New York"),
			("Emily Johnson", 28, "Downtown, Los Angeles"),
			("Michael Brown", 41, "Capitol Hill, Washington D.C.")
		};

		var output = new List<string>();
		var writer = new Writer(s => output.Add(s));

		writer.Array("", () => {
			foreach (var person in originalPersons) {
				writer.Object("", () => {
					writer.String("name", person.name);
					writer.Number("age", person.age);
					writer.String("address", person.address);
				});
			}
		});

		string json = string.Join("", output);

		// Parse it back
		var parsedPersons = new List<(string name, int age, string address)>();
		var reader = new Reader(json);

		while (reader.Next()) {
			if (reader.IsStartArray) {
				reader.Nest();
				do {
					if (reader.IsStartObject) {
						reader.Nest();
						string? name = null;
						int age = 0;
						string? address = null;

						do {
							if (reader.Match("[{name"))
								name = reader.StringValue;
							else if (reader.Match("[{age"))
								age = (int)reader.Number;
							else if (reader.Match("[{address"))
								address = reader.StringValue;
						} while (reader.Next());

						if (name != null && address != null)
							parsedPersons.Add((name, age, address));
					}
				} while (reader.Next());
			}
		}

		Assert.Equal(originalPersons.Count, parsedPersons.Count);
		for (int i = 0; i < originalPersons.Count; i++) {
			Assert.Equal(originalPersons[i].name, parsedPersons[i].name);
			Assert.Equal(originalPersons[i].age, parsedPersons[i].age);
			Assert.Equal(originalPersons[i].address, parsedPersons[i].address);
		}
	}

	[Fact]
	public void Reader_DeepNesting_Test()
	{
		const string json = """
        {
            "unexpected": {
                "values": [
                    "deep",
                    {
                        "nesting": {
                            "level": 6
                        }
                    },
                    "done"
                ]
            }
        }
        """;

		var values = new List<string>();
		var reader = new Reader(json);

		while (reader.Next()) {
			if (reader.Match("{unexpected{values[*")) {
				if (reader.IsValue) {
					values.Add(reader.StringValue);
				} else if (reader.IsStartObject) {
					reader.Nest();
					do {
						if (reader.Match("{unexpected{values[{nesting{level")) {
							values.Add(reader.StringValue);
						}
					} while (reader.Next());
				}
			}
		}

		Assert.Equal(3, values.Count);
		Assert.Equal("deep", values[0]);
		Assert.Equal("6", values[1]);
		Assert.Equal("done", values[2]);
	}
}