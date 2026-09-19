package jstream

import (
	"strings"
	"testing"
)

func TestWriterSimpleObject(t *testing.T) {
	w := NewWriter()
	w.Object("", func() {
		w.WriteString("name", "John Doe")
		w.WriteNumber("age", 30)
		w.WriteString("city", "New York")
	})
	json := w.String()
	if !strings.Contains(json, `"name": "John Doe"`) {
		t.Errorf("unexpected output: %s", json)
	}
	if !strings.Contains(json, `"age": 30`) {
		t.Errorf("unexpected output: %s", json)
	}
}

func TestWriterRoundTrip(t *testing.T) {
	w := NewWriter()
	w.Object("", func() {
		w.WriteString("name", "John Doe")
		w.WriteNumber("age", 30)
		w.Array("cities", func() {
			w.WriteStringValue("New York")
			w.WriteStringValue("London")
			w.WriteStringValue("Tokyo")
		})
	})
	json := w.String()

	reader := NewReader(json)
	var name string
	var age float64
	var cities []string
	for reader.Next() {
		if reader.Match("{name") && reader.IsString() {
			name = reader.StringValue()
		} else if reader.Match("{age") && reader.IsNumber() {
			age = reader.Number()
		} else if reader.Match("{cities[*") && reader.IsString() {
			cities = append(cities, reader.StringValue())
		}
	}
	if name != "John Doe" {
		t.Errorf("name = %q, want John Doe", name)
	}
	if age != 30 {
		t.Errorf("age = %v, want 30", age)
	}
	if len(cities) != 3 {
		t.Fatalf("cities = %v, want 3", cities)
	}
}

func TestWriterRaw(t *testing.T) {
	w := NewWriter()
	w.Object("", func() {
		w.WriteString("name", "test")
		w.WriteRaw("meta", `{"source":"api"}`)
	})
	json := w.String()
	if !strings.Contains(json, `"meta": {"source":"api"}`) {
		t.Errorf("raw output missing: %s", json)
	}
}

func TestWriterCallback(t *testing.T) {
	var parts []string
	w := NewWriterWithOutput(func(s string) { parts = append(parts, s) })
	w.WriteString("hello", "world")
	json := strings.Join(parts, "")
	if !strings.Contains(json, `"hello": "world"`) {
		t.Errorf("unexpected output: %s", json)
	}
}
