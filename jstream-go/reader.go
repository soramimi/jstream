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

	inputCallback       func()
	notEnoughInput      bool
	commentState        byte // 0=none, '/'=line comment, '*'=block comment
	extractionSupport   bool
}

// NewReader creates a new Reader for the given JSON text.
func NewReader(json string) *Reader {
	return &Reader{json: json, extractionSupport: true}
}

// NewReaderWithCallback creates a streaming Reader with the given input callback.
func NewReaderWithCallback(callback func()) *Reader {
	return &Reader{inputCallback: callback, extractionSupport: false}
}

// Input appends incremental input to the internal buffer. It is used in
// streaming mode after NewReaderWithCallback, or directly with NewReader.
func (r *Reader) Input(in string) {
	if in == "" {
		return
	}
	r.notEnoughInput = false
	r.extractionSupport = false
	r.json = r.json[r.pos:] + in
	r.pos = 0
}

// IsNotEnoughInput returns true when parsing was interrupted because the
// buffer ended mid-token in streaming mode.
func (r *Reader) IsNotEnoughInput() bool {
	return r.notEnoughInput
}

// NextDocument clears the EndDocument state so the same Reader can parse the
// next JSON document from the stream.
func (r *Reader) NextDocument() {
	if r.State() == StateEndDocument {
		r.states = nil
	}
}

func (r *Reader) needInput() {
	if r.inputCallback != nil {
		r.inputCallback()
	}
}

func (r *Reader) peekNextChar() int {
	if r.pos < len(r.json) {
		return int(r.json[r.pos])
	}
	r.needInput()
	if r.pos < len(r.json) {
		return int(r.json[r.pos])
	}
	return -1
}

type stateItem struct {
	stateType StateType
	pos       int
}

type nestItem struct {
	depth int
	path  string
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
	if !r.extractionSupport {
		r.pushError("extract() is not supported in streaming input mode")
		return ""
	}
	if r.lastState != nil {
		return r.json[r.lastState.pos:r.pos]
	}
	return ""
}

// ExtractRange returns the raw text between two byte offsets.
func (r *Reader) ExtractRange(begin, end int) string {
	if !r.extractionSupport {
		r.pushError("extract() is not supported in streaming input mode")
		return ""
	}
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
	if r.inputCallback != nil && !r.notEnoughInput {
		if r.State() != StateEndDocument {
			return true
		}
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
	notEnoughInput := true
loop:
	for r.pos < len(r.json) {
		notEnoughInput = false

		if r.skipSpace() {
			continue
		}
		if r.pos >= len(r.json) {
			notEnoughInput = true
			break
		}

		ch := r.json[r.pos]
		switch ch {
		case '}':
			return r.handleEndObject()
		case ']':
			return r.handleEndArray()
		case ',':
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
				continue
			}
			continue
		case '{':
			return r.handleStartObject()
		case '[':
			return r.handleStartArray()
		case '"':
			if r.handleString() {
				return true
			}
			if r.notEnoughInput {
				notEnoughInput = true
				break loop
			}
			return false
		default:
			if r.State() == StateKey || r.IsArray() {
				if (ch >= '0' && ch <= '9') || ch == '-' || ch == '+' || ch == '.' {
					if r.handleNumber() {
						return true
					}
					if r.notEnoughInput {
						notEnoughInput = true
						break loop
					}
					return false
				}
				if (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') {
					if r.handleSymbol() {
						return true
					}
					if r.notEnoughInput {
						notEnoughInput = true
						break loop
					}
					return false
				}
			} else if r.AllowUnquotedKey && ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
				if r.handleUnquotedKey() {
					return true
				}
				if r.notEnoughInput {
					notEnoughInput = true
					break loop
				}
			}
			r.pushError("Syntax error")
			return false
		}
	}

	if (r.State() == StateEndObject || r.State() == StateEndArray) && len(r.depth) == 0 {
		r.pushState(stateItem{stateType: StateEndDocument})
		return false
	}

	if notEnoughInput {
		r.notEnoughInput = true
		r.needInput()
		return false
	}

	return false
}

func isSpace(c byte) bool {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r'
}

func (r *Reader) skipSpace() bool {
	ret := false
	for {
		c := r.peekNextChar()
		if c < 0 {
			if r.commentState != 0 {
				r.notEnoughInput = true
			}
			break
		}
		if r.commentState != 0 {
			if r.commentState == '*' {
				if c == '/' {
					r.commentState = 0
				}
			} else if r.commentState == '/' {
				if c == '\n' || c == '\r' {
					r.commentState = 0
				}
			}
		} else if !isSpace(byte(c)) {
			if r.AllowComment && c == '/' {
				if r.pos+1 >= len(r.json) {
					r.needInput()
				}
				if r.pos+1 < len(r.json) {
					t := r.json[r.pos+1]
					if t == '*' || t == '/' {
						r.commentState = t
						r.pos += 2
						ret = true
						continue
					}
				}
			}
			break
		}
		r.pos++
		ret = true
	}
	return ret
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
	s, n, ok := r.parseString()
	if !ok || r.pos+n == len(r.json) {
		r.notEnoughInput = true
		return false
	}
	r.pos += n
	r.stringValue = s
	r.skipSpace()
	if r.State() == StateKey {
		r.pushState(stateItem{stateType: StateString})
		return true
	}
	c := r.peekNextChar()
	if c < 0 {
		r.notEnoughInput = true
		return false
	}
	if c == ':' {
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
	text, v, n, ok := r.parseNumber()
	if !ok || r.pos+n == len(r.json) {
		r.notEnoughInput = true
		return false
	}
	r.pos += n
	r.stringValue = text
	r.numberValue = v
	r.pushState(stateItem{stateType: StateNumber})
	return true
}

func (r *Reader) handleSymbol() bool {
	sym, n, ok := r.parseSymbol()
	if !ok || r.pos+n == len(r.json) {
		r.notEnoughInput = true
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
	r.pos += n
	r.pushState(stateItem{stateType: st})
	return true
}

func (r *Reader) handleUnquotedKey() bool {
	sym, n, ok := r.parseSymbol()
	if !ok || r.pos+n == len(r.json) {
		r.notEnoughInput = true
		return false
	}
	r.stringValue = sym
	r.pos += n
	r.skipSpace()
	if r.pos >= len(r.json) {
		r.notEnoughInput = true
		return false
	}
	if r.json[r.pos] == ':' {
		r.pos++
		r.key = r.stringValue
		r.pushState(stateItem{stateType: StateKey})
		return true
	}
	return false
}

func (r *Reader) parseSymbol() (string, int, bool) {
	start := r.pos
	p := r.pos
	for p < len(r.json) {
		c := r.json[p]
		if (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' {
			p++
		} else {
			break
		}
	}
	if p > start {
		return r.json[start:p], p - start, true
	}
	return "", 0, false
}

func (r *Reader) parseString() (string, int, bool) {
	if r.pos >= len(r.json) || r.json[r.pos] != '"' {
		return "", 0, false
	}
	start := r.pos
	p := r.pos + 1
	var sb strings.Builder
	for p < len(r.json) {
		c := r.json[p]
		if c == '"' {
			p++
			return sb.String(), p - start, true
		}
		if c == '\\' {
			p++
			if p >= len(r.json) {
				return "", 0, false
			}
			esc := r.json[p]
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
				if p+4 >= len(r.json) {
					return "", 0, false
				}
				hex := r.json[p+1 : p+5]
				code, err := strconv.ParseInt(hex, 16, 32)
				if err != nil {
					r.pushError("invalid unicode escape")
					return "", 0, false
				}
				p += 4
				if code >= 0xD800 && code < 0xDC00 {
					if p+6 >= len(r.json) || r.json[p+1] != '\\' || r.json[p+2] != 'u' {
						return "", 0, false
					}
					lowHex := r.json[p+3 : p+7]
					low, err := strconv.ParseInt(lowHex, 16, 32)
					if err != nil || low < 0xDC00 || low >= 0xE000 {
						return "", 0, false
					}
					p += 6
					code = int64(((int(code)-0xD800)<<10)+(int(low)-0xDC00)+0x10000)
					sb.WriteString(string(rune(code)))
				} else if code >= 0xDC00 && code < 0xE000 {
					return "", 0, false
				} else {
					sb.WriteString(string(rune(code)))
				}
			default:
				sb.WriteByte(esc)
			}
			p++
			continue
		}
		sb.WriteByte(c)
		p++
	}
	return "", 0, false
}

func isHexDigit(c byte) bool {
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')
}

func (r *Reader) parseNumber() (string, float64, int, bool) {
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
					return r.json[start:p], sign * float64(v), p - start, true
				}
			}
		}
	}

	p := r.pos
	for p < len(r.json) {
		c := r.json[p]
		if (c >= '0' && c <= '9') || c == '.' || c == '+' || c == '-' || c == 'e' || c == 'E' {
			p++
		} else {
			break
		}
	}
	if p == start {
		return "", 0, 0, false
	}
	text := r.json[start:p]
	if v, ok := parseNumber(text, false, r.AllowSpecialConstant); ok {
		return text, v, p - start, true
	}
	return "", 0, 0, false
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
