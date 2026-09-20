package jstream

import (
	"strconv"
	"strings"
)

// Error represents a parser error with position information.
type Error struct {
	Message string
	Offset  int
	Line    int
	Column  int
}

func (e Error) Error() string { return e.Message }

// Reader is a streaming pull-based JSON parser.
type Reader struct {
	json   string
	pos    int
	states []stateItem
	depth  []string
	depthStack []nestItem
	key    string
	stringValue string
	numberValue float64
	isArray bool
	hold    bool
	lastState *stateItem
	errors  []Error

	AllowComment         bool
	AllowAmbiguousComma  bool
	AllowUnquotedKey     bool
	AllowHexadecimal     bool
	AllowSpecialConstant bool
	AllowKeyInArray      bool
}

type stateItem struct {
	stateType StateType
	pos       int
}

type nestItem struct {
	depth int
	path  string
}

// NewReader creates a new Reader for the given JSON text.
func NewReader(json string) *Reader {
	return &Reader{json: json}
}

func (r *Reader) State() StateType {
	if len(r.states) == 0 {
		return StateNone
	}
	return r.states[len(r.states)-1].stateType
}

func (r *Reader) HasError() bool { return len(r.errors) > 0 }
func (r *Reader) Errors() []Error { return r.errors }

func (r *Reader) IsStartObject() bool { return r.State() == StateStartObject }
func (r *Reader) IsEndObject() bool   { return r.State() == StateEndObject }
func (r *Reader) IsStartArray() bool  { return r.State() == StateStartArray }
func (r *Reader) IsEndArray() bool    { return r.State() == StateEndArray }

func (r *Reader) IsConstant() bool {
	s := r.State()
	return s == StateString || s == StateNumber || s == StateNull || s == StateFalse || s == StateTrue
}

func (r *Reader) IsStructure() bool {
	s := r.State()
	switch s {
	case StateStartObject, StateStartArray:
		return true
	case StateEndObject, StateEndArray:
		if len(r.states) > 1 {
			prev := r.states[len(r.states)-2].stateType
			return prev == StateStartObject || prev == StateStartArray
		}
	}
	return false
}

func (r *Reader) IsValue() bool { return r.IsConstant() || r.IsStructure() }

func (r *Reader) Key() string         { return r.key }
func (r *Reader) StringValue() string { return r.stringValue }
func (r *Reader) Number() float64     { return r.numberValue }
func (r *Reader) IsArray() bool       { return r.isArray }
func (r *Reader) Depth() int          { return len(r.depth) }
func (r *Reader) Tell() int           { return r.pos }

func (r *Reader) Symbol() StateType {
	s := r.State()
	switch s {
	case StateNull, StateFalse, StateTrue:
		return s
	}
	return StateNone
}

func (r *Reader) IsNull() bool    { return r.Symbol() == StateNull }
func (r *Reader) IsFalse() bool   { return r.Symbol() == StateFalse }
func (r *Reader) IsTrue() bool    { return r.Symbol() == StateTrue }
func (r *Reader) IsBoolean() bool { return r.IsFalse() || r.IsTrue() }
func (r *Reader) BooleanValue() bool { return r.IsTrue() }
func (r *Reader) IsNumber() bool  { return r.State() == StateNumber }
func (r *Reader) IsString() bool  { return r.State() == StateString }

func (r *Reader) Path() string {
	var sb strings.Builder
	for _, s := range r.depth {
		sb.WriteString(s)
	}
	if r.State() == StateStartObject || r.State() == StateStartArray {
		return sb.String()
	}
	return sb.String() + r.key
}

func (r *Reader) Extract() string {
	if r.lastState != nil {
		return r.json[r.lastState.pos:r.pos]
	}
	return ""
}

// ExtractRange returns the raw text between two byte offsets.
func (r *Reader) ExtractRange(begin, end int) string {
	if begin >= 0 && end <= len(r.json) && begin <= end {
		return r.json[begin:end]
	}
	return ""
}

func (r *Reader) Reset() { r.errors = nil }
func (r *Reader) Hold()  { r.hold = true }

func (r *Reader) Nest(callback ...func()) {
	r.depthStack = append(r.depthStack, nestItem{depth: r.Depth(), path: r.Path()})
	if len(callback) > 0 && callback[0] != nil {
		fn := callback[0]
		for {
			fn()
			if !r.Next() {
				break
			}
		}
	}
}

func (r *Reader) Next() bool {
	if r.hold {
		r.hold = false
		return true
	}
	if r.internalNext() {
		if len(r.depthStack) == 0 {
			return true
		}
		if r.Depth() >= r.depthStack[len(r.depthStack)-1].depth {
			return true
		}
		r.depthStack = r.depthStack[:len(r.depthStack)-1]
		r.Hold()
	}
	return false
}

func (r *Reader) GetVariant() Variant {
	switch {
	case r.IsNull():
		return Null
	case r.IsFalse():
		return NewBoolean(false)
	case r.IsTrue():
		return NewBoolean(true)
	case r.IsNumber():
		return NewNumber(r.Number())
	case r.IsString():
		return NewString(r.StringValue())
	}
	return Null
}

func (r *Reader) internalNext() bool {
	for r.pos < len(r.json) {
		r.skipSpace()
		if r.pos >= len(r.json) {
			break
		}
		ch := r.json[r.pos]
		switch ch {
		case '}':
			return r.handleEndObject()
		case ']':
			return r.handleEndArray()
		case ',':
			return r.handleComma()
		case '{':
			return r.handleStartObject()
		case '[':
			return r.handleStartArray()
		case '"':
			return r.handleString()
		default:
			if r.State() == StateKey || r.IsArray() {
				if (ch >= '0' && ch <= '9') || ch == '-' || ch == '+' || ch == '.' {
					return r.handleNumber()
				}
				if (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') {
					return r.handleSymbol()
				}
			} else if r.AllowUnquotedKey && ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
				return r.handleUnquotedKey()
			}
			r.pushError("Syntax error")
			return false
		}
	}
	return false
}

func isSpace(c byte) bool {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r'
}

func (r *Reader) skipSpace() {
	for r.pos < len(r.json) {
		c := r.json[r.pos]
		if isSpace(c) {
			r.pos++
			continue
		}
		if r.AllowComment && c == '/' && r.pos+1 < len(r.json) {
			if r.json[r.pos+1] == '/' {
				r.pos += 2
				for r.pos < len(r.json) && r.json[r.pos] != '\n' && r.json[r.pos] != '\r' {
					r.pos++
				}
				continue
			}
			if r.json[r.pos+1] == '*' {
				r.pos += 2
				for r.pos+1 < len(r.json) {
					if r.json[r.pos] == '*' && r.json[r.pos+1] == '/' {
						r.pos += 2
						break
					}
					r.pos++
				}
				continue
			}
		}
		break
	}
}

func (r *Reader) handleEndObject() bool {
	r.pos++
	r.stringValue = ""
	key := ""
	if len(r.depth) > 0 {
		key = r.depth[len(r.depth)-1]
		if strings.HasSuffix(key, "{") {
			key = key[:len(key)-1]
		}
		r.depth = r.depth[:len(r.depth)-1]
	}
	for {
		wasStartObject := r.State() == StateStartObject
		if !r.popState() {
			break
		}
		if wasStartObject {
			r.pushState(stateItem{stateType: StateEndObject, pos: r.pos})
			r.key = key
			return true
		}
	}
	return false
}

func (r *Reader) handleEndArray() bool {
	r.pos++
	r.stringValue = ""
	key := ""
	if len(r.depth) > 0 {
		key = r.depth[len(r.depth)-1]
		if strings.HasSuffix(key, "[") {
			key = key[:len(key)-1]
		}
		r.depth = r.depth[:len(r.depth)-1]
	}
	for {
		wasStartArray := r.State() == StateStartArray
		if !r.popState() {
			break
		}
		if wasStartArray {
			r.pushState(stateItem{stateType: StateEndArray, pos: r.pos})
			r.key = key
			return true
		}
	}
	return false
}

func (r *Reader) handleComma() bool {
	r.pos++
	if r.State() == StateKey {
		r.pushState(stateItem{stateType: StateNull})
		return true
	}
	r.skipSpace()
	if r.IsStructure() || r.IsValue() {
		r.popState()
	}
	r.pushState(stateItem{stateType: StateComma})
	if r.AllowAmbiguousComma {
		return r.Next()
	}
	return true
}

func (r *Reader) handleStartObject() bool {
	pos := r.pos
	r.pos++
	if r.State() != StateKey {
		r.key = ""
		r.stringValue = ""
	}
	r.depth = append(r.depth, r.key+"{")
	r.pushState(stateItem{stateType: StateStartObject, pos: pos})
	return true
}

func (r *Reader) handleStartArray() bool {
	pos := r.pos
	r.pos++
	if r.State() != StateKey {
		r.key = ""
		r.stringValue = ""
	}
	r.depth = append(r.depth, r.key+"[")
	r.pushState(stateItem{stateType: StateStartArray, pos: pos})
	return true
}

func (r *Reader) handleString() bool {
	s, ok := r.parseString()
	if !ok {
		r.pushError("Invalid string")
		return false
	}
	r.stringValue = s
	if r.State() == StateKey {
		r.pushState(stateItem{stateType: StateString})
		return true
	}
	r.skipSpace()
	if r.pos < len(r.json) && r.json[r.pos] == ':' {
		if r.IsArray() {
			if !r.AllowKeyInArray {
				r.pushError("Unexpected key in array")
				return false
			}
		}
		r.pos++
		r.key = r.stringValue
		r.pushState(stateItem{stateType: StateKey})
		return true
	}
	r.pushState(stateItem{stateType: StateString})
	return true
}

func (r *Reader) handleNumber() bool {
	text, v, ok := r.parseNumber()
	if !ok {
		return false
	}
	r.stringValue = text
	r.numberValue = v
	r.pushState(stateItem{stateType: StateNumber})
	return true
}

func (r *Reader) handleSymbol() bool {
	sym, ok := r.parseSymbol()
	if !ok {
		return false
	}
	r.stringValue = sym
	var st StateType
	switch sym {
	case "false":
		st = StateFalse
	case "true":
		st = StateTrue
	case "null":
		st = StateNull
	default:
		return false
	}
	r.pushState(stateItem{stateType: st})
	return true
}

func (r *Reader) handleUnquotedKey() bool {
	sym, ok := r.parseSymbol()
	if !ok {
		return false
	}
	r.stringValue = sym
	endPos := r.pos
	tempPos := endPos
	for tempPos < len(r.json) && isSpace(r.json[tempPos]) {
		tempPos++
	}
	if tempPos < len(r.json) && r.json[tempPos] == ':' {
		r.pos = tempPos + 1
		r.key = r.stringValue
		r.pushState(stateItem{stateType: StateKey})
		return true
	}
	return false
}

func (r *Reader) parseSymbol() (string, bool) {
	start := r.pos
	for r.pos < len(r.json) {
		c := r.json[r.pos]
		if (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' {
			r.pos++
		} else {
			break
		}
	}
	if r.pos > start {
		return r.json[start:r.pos], true
	}
	return "", false
}

func (r *Reader) parseString() (string, bool) {
	if r.pos >= len(r.json) || r.json[r.pos] != '"' {
		return "", false
	}
	r.pos++
	var sb strings.Builder
	for r.pos < len(r.json) {
		c := r.json[r.pos]
		if c == '"' {
			r.pos++
			return sb.String(), true
		}
		if c == '\\' {
			r.pos++
			if r.pos >= len(r.json) {
				break
			}
			esc := r.json[r.pos]
			switch esc {
			case 'b':
				sb.WriteByte('\b')
			case 'n':
				sb.WriteByte('\n')
			case 'r':
				sb.WriteByte('\r')
			case 'f':
				sb.WriteByte('\f')
			case 't':
				sb.WriteByte('\t')
			case 'v':
				sb.WriteByte('\v')
			case '\\', '"':
				sb.WriteByte(esc)
			case 'u':
				if r.pos+4 >= len(r.json) {
					return "", false
				}
				hex := r.json[r.pos+1 : r.pos+5]
				code, err := strconv.ParseInt(hex, 16, 32)
				if err != nil {
					return "", false
				}
				r.pos += 4
				if code >= 0xD800 && code < 0xDC00 {
					if r.pos+6 >= len(r.json) || r.json[r.pos+1] != '\\' || r.json[r.pos+2] != 'u' {
						return "", false
					}
					lowHex := r.json[r.pos+3 : r.pos+7]
					low, err := strconv.ParseInt(lowHex, 16, 32)
					if err != nil || low < 0xDC00 || low >= 0xE000 {
						return "", false
					}
					r.pos += 6
					code = int64(((int(code)-0xD800)<<10)+(int(low)-0xDC00)+0x10000)
					sb.WriteString(string(rune(code)))
				} else if code >= 0xDC00 && code < 0xE000 {
					return "", false
				} else {
					sb.WriteString(string(rune(code)))
				}
			default:
				sb.WriteByte(esc)
			}
			r.pos++
			continue
		}
		sb.WriteByte(c)
		r.pos++
	}
	return "", false
}

func isHexDigit(c byte) bool {
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')
}

func (r *Reader) parseNumber() (string, float64, bool) {
	start := r.pos

	if r.AllowHexadecimal {
		p := r.pos
		sign := 1.0
		if p < len(r.json) && r.json[p] == '-' {
			sign = -1
			p++
		}
		if p+1 < len(r.json) && r.json[p] == '0' && (r.json[p+1] == 'x' || r.json[p+1] == 'X') {
			p += 2
			hexStart := p
			for p < len(r.json) && isHexDigit(r.json[p]) {
				p++
			}
			if p > hexStart {
				if v, err := strconv.ParseInt(r.json[hexStart:p], 16, 64); err == nil {
					r.pos = p
					return r.json[start:r.pos], sign * float64(v), true
				}
			}
		}
	}

	for r.pos < len(r.json) {
		c := r.json[r.pos]
		if (c >= '0' && c <= '9') || c == '.' || c == '+' || c == '-' || c == 'e' || c == 'E' {
			r.pos++
		} else {
			break
		}
	}
	if start == r.pos {
		return "", 0, false
	}
	text := r.json[start:r.pos]
	if v, ok := parseNumber(text, false, r.AllowSpecialConstant); ok {
		return text, v, true
	}
	return "", 0, false
}

func (r *Reader) pushState(s stateItem) {
	state := r.State()
	if state == StateKey || state == StateComma || state == StateEndObject {
		r.states = r.states[:len(r.states)-1]
	}
	r.states = append(r.states, s)
	if r.IsArray() {
		r.key = ""
	}
	switch s.stateType {
	case StateStartArray:
		r.isArray = true
	case StateStartObject, StateKey:
		r.isArray = false
	}
}

func (r *Reader) popState() bool {
	if len(r.states) == 0 {
		return false
	}
	last := r.states[len(r.states)-1]
	r.lastState = &last
	r.states = r.states[:len(r.states)-1]

	r.isArray = false
	for i := len(r.states) - 1; i >= 0; i-- {
		switch r.states[i].stateType {
		case StateStartArray:
			r.isArray = true
			i = -1
		case StateStartObject, StateKey:
			r.isArray = false
			i = -1
		}
	}

	if r.State() == StateKey {
		r.states = r.states[:len(r.states)-1]
	}
	r.key = ""
	return true
}

func (r *Reader) pushError(msg string) {
	r.states = nil
	offset := r.pos
	line, column := 1, 1
	for i := 0; i < r.pos && i < len(r.json); i++ {
		if r.json[i] == '\n' {
			line++
			column = 1
		} else if r.json[i] != '\r' {
			column++
		}
	}
	r.errors = append(r.errors, Error{Message: msg, Offset: offset, Line: line, Column: column})
}

// Match reports whether the current event matches the given path expression.
func (r *Reader) Match(path string, matchEndStructure ...bool) bool {
	matchEnd := false
	if len(matchEndStructure) > 0 {
		matchEnd = matchEndStructure[0]
	}
	if !r.IsValue() {
		return false
	}
	if len(path) > 0 && path[0] == '@' {
		if len(r.depthStack) > 0 {
			item := r.depthStack[len(r.depthStack)-1]
			path = item.path + path[1:]
		}
	}
	pathPos := 0
	pathChar := func(i int) byte {
		if i < len(path) {
			return path[i]
		}
		return 0
	}
	for i := 0; i < len(r.depth); i++ {
		element := r.depth[i]
		if element == "" {
			return false
		}
		if pathChar(pathPos) == '*' {
			if pathChar(pathPos+1) == '*' {
				if pathChar(pathPos+2) == 0 {
					return true
				}
				return false
			}
			c := element[len(element)-1]
			if c == '{' || c == '[' {
				if pathChar(pathPos+1) == c {
					pathPos += 2
					continue
				}
				if pathChar(pathPos+1) == 0 {
					if i+1 == len(r.depth) {
						if c == '{' && r.State() == StateStartObject {
							return true
						}
						if c == '[' && r.State() == StateStartArray {
							return true
						}
					}
					return false
				}
			}
		}
		if pathPos+len(element) > len(path) {
			return false
		}
		if path[pathPos:pathPos+len(element)] != element {
			return false
		}
		pathPos += len(element)
	}
	if pathChar(pathPos) == '*' {
		if pathChar(pathPos+1) == '*' && pathChar(pathPos+2) == 0 {
			return true
		}
		if pathChar(pathPos+1) == 0 {
			if r.IsConstant() {
				return true
			}
			if matchEnd && (r.State() == StateEndObject || r.State() == StateEndArray) {
				return true
			}
			return false
		}
	}
	remaining := ""
	if pathPos < len(path) {
		remaining = path[pathPos:]
	}
	return remaining == r.key
}

// MatchStartObject reports whether the current event is the start of an object at path.
func (r *Reader) MatchStartObject(path string) bool {
	return r.State() == StateStartObject && r.Match(path)
}

// MatchEndObject reports whether the current event is the end of an object at path.
func (r *Reader) MatchEndObject(path string) bool {
	return r.State() == StateEndObject && r.Match(path, true)
}

// MatchStartArray reports whether the current event is the start of an array at path.
func (r *Reader) MatchStartArray(path string) bool {
	return r.State() == StateStartArray && r.Match(path)
}

// MatchEndArray reports whether the current event is the end of an array at path.
func (r *Reader) MatchEndArray(path string) bool {
	return r.State() == StateEndArray && r.Match(path, true)
}
