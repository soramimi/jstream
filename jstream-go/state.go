package jstream

// StateType represents the type of a parsing event.
type StateType int

const (
	// Symbols
	StateNone StateType = iota
	StateNull
	StateFalse
	StateTrue
	// States
	StateKey         StateType = 100
	StateComma                 = 101
	StateStartObject           = 102
	StateEndObject             = 103
	StateStartArray            = 104
	StateEndArray              = 105
	StateString                = 106
	StateNumber                = 107
)
