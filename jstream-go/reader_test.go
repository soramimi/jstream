package jstream

import (
	"testing"
)

func TestReaderSimpleObject(t *testing.T) {
	json := `{
		"name": "John",
		"age": 30,
		"city": "New York"
	}`
	reader := NewReader(json)
	var name string
	var age float64
	var city string
	for reader.Next() {
		if reader.Match("{name") && reader.IsString() {
			name = reader.StringValue()
		} else if reader.Match("{age") && reader.IsNumber() {
			age = reader.Number()
		} else if reader.Match("{city") && reader.IsString() {
			city = reader.StringValue()
		}
	}
	if name != "John" {
		t.Errorf("name = %q, want John", name)
	}
	if age != 30 {
		t.Errorf("age = %v, want 30", age)
	}
	if city != "New York" {
		t.Errorf("city = %q, want New York", city)
	}
}

func TestReaderArray(t *testing.T) {
	json := `{"a": [12, 34, 56, 78]}`
	reader := NewReader(json)
	var numbers []float64
	for reader.Next() {
		if reader.Match("{a[*") && reader.IsNumber() {
			numbers = append(numbers, reader.Number())
		}
	}
	if len(numbers) != 4 {
		t.Fatalf("len(numbers) = %d, want 4", len(numbers))
	}
	for i, want := range []float64{12, 34, 56, 78} {
		if numbers[i] != want {
			t.Errorf("numbers[%d] = %v, want %v", i, numbers[i], want)
		}
	}
}

func TestReaderUnicodeEscape(t *testing.T) {
	json := `{"a": "\u3042", "b": "\ud834\udd1e"}`
	reader := NewReader(json)
	var a, b string
	for reader.Next() {
		if reader.Match("{a") && reader.IsString() {
			a = reader.StringValue()
		} else if reader.Match("{b") && reader.IsString() {
			b = reader.StringValue()
		}
	}
	if a != "あ" {
		t.Errorf("a = %q, want あ", a)
	}
	if b != "𝄞" {
		t.Errorf("b = %q, want 𝄞", b)
	}
}

func TestReaderErrorPosition(t *testing.T) {
	json := `{"foo": "bar", "bad": "\u00zz"}`
	reader := NewReader(json)
	for reader.Next() {
	}
	if !reader.HasError() {
		t.Fatal("expected error")
	}
	err := reader.Errors()[0]
	if err.Line != 1 {
		t.Errorf("line = %d, want 1", err.Line)
	}
}

func TestReaderHexNumber(t *testing.T) {
	json := `{"a": -0xFF}`
	reader := NewReader(json)
	reader.AllowHexadecimal = true
	var v float64
	for reader.Next() {
		if reader.Match("{a") && reader.IsNumber() {
			v = reader.Number()
		}
	}
	if v != -255 {
		t.Errorf("v = %v, want -255", v)
	}
}

func TestReaderMatchStartObject(t *testing.T) {
	json := `[{"name": "x"}, {"name": "y"}]`
	reader := NewReader(json)
	count := 0
	for reader.Next() {
		if reader.MatchStartObject("[*{") {
			count++
		}
	}
	if count != 2 {
		t.Errorf("count = %d, want 2", count)
	}
}

func TestReaderAtRelativePath(t *testing.T) {
	json := `{
		"array": [
			{"value": 123},
			456
		]
	}`
	reader := NewReader(json)
	var values []float64
	for reader.Next() {
		if reader.Match("{array[**") {
			reader.Nest(func() {
				if reader.Match("@{value") {
					values = append(values, reader.Number())
				} else if reader.IsConstant() {
					values = append(values, reader.Number())
				}
			})
		}
	}
	if len(values) != 2 {
		t.Fatalf("len(values) = %d, want 2", len(values))
	}
	if values[0] != 123 {
		t.Errorf("values[0] = %v, want 123", values[0])
	}
	if values[1] != 456 {
		t.Errorf("values[1] = %v, want 456", values[1])
	}
}

type parsedData struct {
	name    string
	age     float64
	city    string
	street  string
	zip     string
}

func TestStreaming0(t *testing.T) {
	json1 := "\n{\n\t\"name\": \"John\",\n\t\"age\": 30,\n\t\"city\": \"New Yo"
	json2 := `rk",
		"address": {
			"street": "123 Main St",
			"zip": "10001"
		}
}
`
	var parsed parsedData
	reader := NewReader("")
	reader.Input(json1)
	reader.Input(json2)
	for reader.Next() {
		if reader.Match("{name") && reader.IsString() {
			parsed.name = reader.StringValue()
		} else if reader.Match("{age") && reader.IsNumber() {
			parsed.age = reader.Number()
		} else if reader.Match("{city") && reader.IsString() {
			parsed.city = reader.StringValue()
		} else if reader.Match("{address{street") && reader.IsString() {
			parsed.street = reader.StringValue()
		} else if reader.Match("{address{zip") && reader.IsString() {
			parsed.zip = reader.StringValue()
		}
	}
	if parsed.name != "John" {
		t.Errorf("name = %q, want John", parsed.name)
	}
	if parsed.age != 30 {
		t.Errorf("age = %v, want 30", parsed.age)
	}
	if parsed.city != "New York" {
		t.Errorf("city = %q, want New York", parsed.city)
	}
	if parsed.street != "123 Main St" {
		t.Errorf("street = %q, want 123 Main St", parsed.street)
	}
	if parsed.zip != "10001" {
		t.Errorf("zip = %q, want 10001", parsed.zip)
	}
}

func TestStreaming1(t *testing.T) {
	json := "\n{\n\t\"name\": \"John\",\n\t\"age\": 3"
	var parsed parsedData
	reader := NewReader("")
	reader.Input(json)
	for reader.Next() {
		if reader.Match("{name") && reader.IsString() {
			parsed.name = reader.StringValue()
		} else if reader.Match("{age") && reader.IsNumber() {
			parsed.age = reader.Number()
		} else if reader.Match("{city") && reader.IsString() {
			parsed.city = reader.StringValue()
		}
	}
	if parsed.name != "John" {
		t.Errorf("name = %q, want John", parsed.name)
	}
	if parsed.age != 0 {
		t.Errorf("age = %v, want 0", parsed.age)
	}
	if !reader.IsNotEnoughInput() {
		t.Error("expected IsNotEnoughInput to be true")
	}
}

func TestStreaming2(t *testing.T) {
	json := "\n{\n\t\"name\": \"John\",\n\t\"age\": 30,\n\t\"city\": \"New Yo"
	var parsed parsedData
	reader := NewReader("")
	reader.Input(json)
	for reader.Next() {
		if reader.Match("{name") && reader.IsString() {
			parsed.name = reader.StringValue()
		} else if reader.Match("{age") && reader.IsNumber() {
			parsed.age = reader.Number()
		} else if reader.Match("{city") && reader.IsString() {
			parsed.city = reader.StringValue()
		}
	}
	if parsed.name != "John" {
		t.Errorf("name = %q, want John", parsed.name)
	}
	if parsed.age != 30 {
		t.Errorf("age = %v, want 30", parsed.age)
	}
	if parsed.city != "" {
		t.Errorf("city = %q, want empty", parsed.city)
	}
	if !reader.IsNotEnoughInput() {
		t.Error("expected IsNotEnoughInput to be true")
	}
}

func TestStreaming3(t *testing.T) {
	json := "\n{\n\t\"name\": \"John\",\n\t\"age\": 30,\n\t\"city\": \"New York\",\n\t\"address\": {\n\t\t\"street\": \"123 Main St\",\n\t\t\"zip\": \"10001\"\n\t}\n}\n"
	var parsed parsedData
	offset := 0
	var reader *Reader
	reader = NewReaderWithCallback(func() {
		if offset < len(json) {
			reader.Input(json[offset : offset+1])
			offset++
		}
	})
	for reader.Next() {
		if reader.Match("{name") && reader.IsString() {
			parsed.name = reader.StringValue()
		} else if reader.Match("{age") && reader.IsNumber() {
			parsed.age = reader.Number()
		} else if reader.Match("{city") && reader.IsString() {
			parsed.city = reader.StringValue()
		} else if reader.Match("{address{street") && reader.IsString() {
			parsed.street = reader.StringValue()
		} else if reader.Match("{address{zip") && reader.IsString() {
			parsed.zip = reader.StringValue()
		}
	}
	if parsed.name != "John" {
		t.Errorf("name = %q, want John", parsed.name)
	}
	if parsed.age != 30 {
		t.Errorf("age = %v, want 30", parsed.age)
	}
	if parsed.city != "New York" {
		t.Errorf("city = %q, want New York", parsed.city)
	}
	if parsed.street != "123 Main St" {
		t.Errorf("street = %q, want 123 Main St", parsed.street)
	}
	if parsed.zip != "10001" {
		t.Errorf("zip = %q, want 10001", parsed.zip)
	}
}

func TestStreaming4(t *testing.T) {
	json := "\n{\n\t\"name\": \"John\",\n\t\"age\": 30,\n\t\"city\": \"New York\", // comment1\n\t\"address\": {\n\t\t\"street\": \"123 Main St\", /* comment2 */\n\t\t\"zip\": \"10001\"\n\t}\n}\n"
	var parsed parsedData
	offset := 0
	var reader *Reader
	reader = NewReaderWithCallback(func() {
		if offset < len(json) {
			reader.Input(json[offset : offset+1])
			offset++
		}
	})
	reader.AllowComment = true
	for reader.Next() {
		if reader.Match("{name") && reader.IsString() {
			parsed.name = reader.StringValue()
		} else if reader.Match("{age") && reader.IsNumber() {
			parsed.age = reader.Number()
		} else if reader.Match("{city") && reader.IsString() {
			parsed.city = reader.StringValue()
		} else if reader.Match("{address{street") && reader.IsString() {
			parsed.street = reader.StringValue()
		} else if reader.Match("{address{zip") && reader.IsString() {
			parsed.zip = reader.StringValue()
		}
	}
	if parsed.name != "John" {
		t.Errorf("name = %q, want John", parsed.name)
	}
	if parsed.age != 30 {
		t.Errorf("age = %v, want 30", parsed.age)
	}
	if parsed.city != "New York" {
		t.Errorf("city = %q, want New York", parsed.city)
	}
	if parsed.street != "123 Main St" {
		t.Errorf("street = %q, want 123 Main St", parsed.street)
	}
	if parsed.zip != "10001" {
		t.Errorf("zip = %q, want 10001", parsed.zip)
	}
}

func TestStreaming5(t *testing.T) {
	json := "\n{\n\t\"name\": \"John\",\n\t\"age\": 30,\n\t\"city\": \"New York\"\n}\n\n{\n\t\"name\": \"Alice\",\n\t\"age\": 10,\n\t\"city\": \"Wonderland\"\n}\n"
	var parsed parsedData
	offset := 0
	var reader *Reader
	reader = NewReaderWithCallback(func() {
		if offset < len(json) {
			reader.Input(json[offset : offset+1])
			offset++
		}
	})

	parse := func() {
		for reader.Next() {
			if reader.Match("{name") && reader.IsString() {
				parsed.name = reader.StringValue()
			} else if reader.Match("{age") && reader.IsNumber() {
				parsed.age = reader.Number()
			} else if reader.Match("{city") && reader.IsString() {
				parsed.city = reader.StringValue()
			}
		}
	}

	parse()
	if parsed.name != "John" {
		t.Errorf("first name = %q, want John", parsed.name)
	}
	if parsed.age != 30 {
		t.Errorf("first age = %v, want 30", parsed.age)
	}
	if parsed.city != "New York" {
		t.Errorf("first city = %q, want New York", parsed.city)
	}

	reader.NextDocument()

	parse()
	if parsed.name != "Alice" {
		t.Errorf("second name = %q, want Alice", parsed.name)
	}
	if parsed.age != 10 {
		t.Errorf("second age = %v, want 10", parsed.age)
	}
	if parsed.city != "Wonderland" {
		t.Errorf("second city = %q, want Wonderland", parsed.city)
	}
}

func TestStreamingCommentSpanChunks(t *testing.T) {
	// Block comment split across two input() calls.
	{
		reader := NewReader("")
		reader.AllowComment = true
		reader.Input(`{"a": /* comm`)
		reader.Input(`ent */ 1}`)
		a := 0.0
		for reader.Next() {
			if reader.Match("{a") && reader.IsNumber() {
				a = reader.Number()
			}
		}
		if a != 1 {
			t.Errorf("a = %v, want 1", a)
		}
		if reader.HasError() {
			t.Errorf("unexpected error: %v", reader.Errors())
		}
	}

	// Line comment split across two input() calls.
	{
		reader := NewReader("")
		reader.AllowComment = true
		reader.Input("{\"a\": 1 // first lin")
		reader.Input("e\n,\"b\":2}\n")
		a, b := 0.0, 0.0
		for reader.Next() {
			if reader.Match("{a") && reader.IsNumber() {
				a = reader.Number()
			} else if reader.Match("{b") && reader.IsNumber() {
				b = reader.Number()
			}
		}
		if a != 1 {
			t.Errorf("a = %v, want 1", a)
		}
		if b != 2 {
			t.Errorf("b = %v, want 2", b)
		}
		if reader.HasError() {
			t.Errorf("unexpected error: %v", reader.Errors())
		}
	}
}
