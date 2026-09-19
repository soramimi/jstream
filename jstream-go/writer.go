package jstream

import "strings"

// Writer is a streaming JSON generator.
type Writer struct {
	output        func(string)
	sb            strings.Builder
	stack         []int
	enableIndent  bool
	enableNewline bool
	allowNaN      bool
}

// NewWriter creates a Writer that collects output in a string.
func NewWriter() *Writer {
	return &Writer{
		stack:         []int{0},
		enableIndent:  true,
		enableNewline: true,
	}
}

// NewWriterWithOutput creates a Writer that writes chunks to the given callback.
func NewWriterWithOutput(output func(string)) *Writer {
	w := NewWriter()
	w.output = output
	return w
}

func (w *Writer) EnableIndent(enabled bool)  { w.enableIndent = enabled }
func (w *Writer) EnableNewline(enabled bool) { w.enableNewline = enabled }
func (w *Writer) AllowNaN(allow bool)        { w.allowNaN = allow }

func (w *Writer) print(s string) {
	if w.output != nil {
		w.output(s)
	} else {
		w.sb.WriteString(s)
	}
}

func (w *Writer) printNewline() {
	if !w.enableNewline {
		return
	}
	w.print("\n")
}

func (w *Writer) printIndent() {
	if !w.enableIndent {
		return
	}
	depth := len(w.stack) - 1
	for i := 0; i < depth; i++ {
		w.print("  ")
	}
}

func (w *Writer) printString(s string) {
	w.print("\"" + encodeJSONString(s) + "\"")
}

func (w *Writer) printNumber(v float64) bool {
	s := formatNumber(v, w.allowNaN)
	if s == "null" && !w.allowNaN {
		w.print("null")
		return false
	}
	w.print(s)
	return true
}

func (w *Writer) printValue(name string, fn func() bool) bool {
	w.printName(name)
	ok := fn()
	if len(w.stack) > 0 {
		w.stack[len(w.stack)-1]++
	}
	if len(w.stack) == 1 {
		w.flush()
	}
	return ok
}

// WriteName writes a property name (including comma and indentation).
func (w *Writer) WriteName(name string) {
	w.printName(name)
}

func (w *Writer) printName(name string) {
	if len(w.stack) > 0 && w.stack[len(w.stack)-1] > 0 {
		w.print(",")
	}
	if len(w.stack) > 1 {
		w.printNewline()
	}
	w.printIndent()
	if name != "" {
		w.printString(name)
		w.print(":")
		if w.enableIndent {
			w.print(" ")
		}
	}
}

func (w *Writer) printObject(name string, fn func()) {
	w.printName(name)
	w.print("{")
	w.stack = append(w.stack, 0)
	if fn != nil {
		fn()
		w.EndObject()
	}
}

func (w *Writer) printArray(name string, fn func()) {
	w.printName(name)
	w.print("[")
	w.stack = append(w.stack, 0)
	if fn != nil {
		fn()
		w.EndArray()
	}
}

func (w *Writer) endBlock() {
	w.printNewline()
	if len(w.stack) > 0 {
		w.stack = w.stack[:len(w.stack)-1]
		if len(w.stack) > 0 {
			w.stack[len(w.stack)-1]++
		}
	}
	w.printIndent()
}

func (w *Writer) flush() {
	if len(w.stack) > 0 && w.stack[0] > 0 {
		w.printNewline()
	}
	w.stack = []int{0}
}

// StartObject begins a new object.
func (w *Writer) StartObject(name ...string) {
	n := ""
	if len(name) > 0 {
		n = name[0]
	}
	w.printObject(n, nil)
}

// EndObject ends the current object.
func (w *Writer) EndObject() {
	w.endBlock()
	w.print("}")
	if len(w.stack) == 1 {
		w.flush()
	}
}

// Object writes a named object using the provided callback.
func (w *Writer) Object(name string, fn func()) {
	w.printObject(name, fn)
}

// StartArray begins a new array.
func (w *Writer) StartArray(name ...string) {
	n := ""
	if len(name) > 0 {
		n = name[0]
	}
	w.printArray(n, nil)
}

// EndArray ends the current array.
func (w *Writer) EndArray() {
	w.endBlock()
	w.print("]")
	if len(w.stack) == 1 {
		w.flush()
	}
}

// Array writes a named array using the provided callback.
func (w *Writer) Array(name string, fn func()) {
	w.printArray(name, fn)
}

// WriteNumber writes a number value.
func (w *Writer) WriteNumber(name string, value float64) bool {
	return w.printValue(name, func() bool { return w.printNumber(value) })
}

// WriteNumberValue writes an unnamed number value.
func (w *Writer) WriteNumberValue(value float64) bool {
	return w.WriteNumber("", value)
}

// WriteString writes a string value.
func (w *Writer) WriteString(name string, value string) {
	w.printValue(name, func() bool {
		w.printString(value)
		return true
	})
}

// WriteStringValue writes an unnamed string value.
func (w *Writer) WriteStringValue(value string) {
	w.WriteString("", value)
}

// WriteSymbol writes a symbolic value (false, true, or null).
func (w *Writer) WriteSymbol(name string, value StateType) {
	w.printValue(name, func() bool {
		switch value {
		case StateFalse:
			w.print("false")
		case StateTrue:
			w.print("true")
		default:
			w.print("null")
		}
		return true
	})
}

// WriteBoolean writes a boolean value.
func (w *Writer) WriteBoolean(name string, value bool) {
	if value {
		w.WriteSymbol(name, StateTrue)
	} else {
		w.WriteSymbol(name, StateFalse)
	}
}

// WriteBooleanValue writes an unnamed boolean value.
func (w *Writer) WriteBooleanValue(value bool) {
	w.WriteBoolean("", value)
}

// WriteNull writes a null value.
func (w *Writer) WriteNull(name ...string) {
	n := ""
	if len(name) > 0 {
		n = name[0]
	}
	w.WriteSymbol(n, StateNull)
}

// WriteRaw writes raw JSON text. The caller is responsible for its validity.
func (w *Writer) WriteRaw(name string, value string) {
	w.printValue(name, func() bool {
		w.print(value)
		return true
	})
}

// WriteRawValue writes unnamed raw JSON text.
func (w *Writer) WriteRawValue(value string) {
	w.WriteRaw("", value)
}

// Finish writes a final newline if content was produced.
func (w *Writer) Finish() {
	if len(w.stack) > 0 && w.stack[0] > 0 {
		w.printNewline()
	}
}

// String returns the collected output.
func (w *Writer) String() string {
	return w.sb.String()
}
