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
