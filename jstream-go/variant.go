package jstream

import (
	"fmt"
	"math"
)

// Variant represents a JSON value.
// It wraps one of: nil, bool, float64, string, *JObject, *JArray.
type Variant struct {
	value any
}

// NewVariant creates a new null Variant.
func NewVariant() Variant { return Variant{} }

// Null is the JSON null value.
var Null = Variant{}

// NewNull creates a null Variant.
func NewNull() Variant { return Variant{} }

// NewBoolean creates a boolean Variant.
func NewBoolean(v bool) Variant { return Variant{value: v} }

// NewNumber creates a number Variant.
func NewNumber(v float64) Variant { return Variant{value: v} }

// NewString creates a string Variant.
func NewString(v string) Variant { return Variant{value: v} }

// NewObject creates an object Variant.
func NewObject() Variant { return Variant{value: NewJObject()} }

// NewArray creates an array Variant.
func NewArray() Variant { return Variant{value: NewJArray()} }

func (v Variant) IsNull() bool     { return v.value == nil }
func (v Variant) IsBoolean() bool  { _, ok := v.value.(bool); return ok }
func (v Variant) IsNumber() bool   { _, ok := v.value.(float64); return ok }
func (v Variant) IsString() bool   { _, ok := v.value.(string); return ok }
func (v Variant) IsObject() bool   { _, ok := v.value.(*JObject); return ok }
func (v Variant) IsArray() bool    { _, ok := v.value.(*JArray); return ok }
func (v Variant) IsNaN() bool      { f, ok := v.value.(float64); return ok && math.IsNaN(f) }
func (v Variant) IsInfinite() bool { f, ok := v.value.(float64); return ok && math.IsInf(f, 0) }

func (v *Variant) AsObject() *JObject {
	if obj, ok := v.value.(*JObject); ok {
		return obj
	}
	obj := NewJObject()
	v.value = obj
	return obj
}

func (v *Variant) AsArray() *JArray {
	if arr, ok := v.value.(*JArray); ok {
		return arr
	}
	arr := NewJArray()
	v.value = arr
	return arr
}

func getValue[T any](v Variant) (T, bool) {
	var zero T
	if val, ok := v.value.(T); ok {
		return val, true
	}
	return zero, false
}

func (v Variant) Boolean() bool {
	if val, ok := getValue[bool](v); ok {
		return val
	}
	panic(fmt.Sprintf("Variant is not a boolean: %T", v.value))
}

func (v Variant) Number() float64 {
	if val, ok := getValue[float64](v); ok {
		return val
	}
	panic(fmt.Sprintf("Variant is not a number: %T", v.value))
}

func (v Variant) String() string {
	if val, ok := getValue[string](v); ok {
		return val
	}
	panic(fmt.Sprintf("Variant is not a string: %T", v.value))
}

func (v Variant) Object() *JObject {
	if val, ok := getValue[*JObject](v); ok {
		return val
	}
	panic(fmt.Sprintf("Variant is not an object: %T", v.value))
}

func (v Variant) Array() *JArray {
	if val, ok := getValue[*JArray](v); ok {
		return val
	}
	panic(fmt.Sprintf("Variant is not an array: %T", v.value))
}

func (v Variant) TryBoolean() (bool, bool)     { return getValue[bool](v) }
func (v Variant) TryNumber() (float64, bool)   { return getValue[float64](v) }
func (v Variant) TryString() (string, bool)    { return getValue[string](v) }
func (v Variant) TryObject() (*JObject, bool)  { return getValue[*JObject](v) }
func (v Variant) TryArray() (*JArray, bool)    { return getValue[*JArray](v) }

// JObject is a JSON object backed by a map.
type JObject struct {
	m map[string]Variant
}

// NewJObject creates a new empty JSON object.
func NewJObject() *JObject {
	return &JObject{m: make(map[string]Variant)}
}

// Set stores a value under key.
func (o *JObject) Set(key string, value Variant) {
	o.m[key] = value
}

// Get returns the value for key, or Null if missing.
func (o *JObject) Get(key string) Variant {
	if v, ok := o.m[key]; ok {
		return v
	}
	return Null
}

// Has reports whether key exists.
func (o *JObject) Has(key string) bool {
	_, ok := o.m[key]
	return ok
}

// Len returns the number of members.
func (o *JObject) Len() int { return len(o.m) }

// Range calls fn for each key/value pair.
func (o *JObject) Range(fn func(key string, value Variant) bool) {
	for k, v := range o.m {
		if !fn(k, v) {
			break
		}
	}
}

// JArray is a JSON array backed by a slice.
type JArray struct {
	a []Variant
}

// NewJArray creates a new empty JSON array.
func NewJArray() *JArray {
	return &JArray{}
}

// Append adds a value to the array.
func (a *JArray) Append(value Variant) {
	a.a = append(a.a, value)
}

// Get returns the value at index.
func (a *JArray) Get(index int) Variant {
	return a.a[index]
}

// Len returns the number of elements.
func (a *JArray) Len() int { return len(a.a) }

// Range calls fn for each element.
func (a *JArray) Range(fn func(value Variant) bool) {
	for _, v := range a.a {
		if !fn(v) {
			break
		}
	}
}
