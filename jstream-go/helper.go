package jstream

import (
	"fmt"
	"math"
	"strconv"
	"strings"
	"unicode/utf8"
)

func encodeJSONString(s string) string {
	var b strings.Builder
	for i := 0; i < len(s); {
		r, size := utf8.DecodeRuneInString(s[i:])
		if r == utf8.RuneError && size == 1 {
			// Invalid UTF-8 byte: escape as \u00XX.
			b.WriteString(fmt.Sprintf("\\u%04X", s[i]))
			i++
			continue
		}
		switch r {
		case '"':
			b.WriteString(`\"`)
		case '\\':
			b.WriteString(`\\`)
		case '\b':
			b.WriteString(`\b`)
		case '\f':
			b.WriteString(`\f`)
		case '\n':
			b.WriteString(`\n`)
		case '\r':
			b.WriteString(`\r`)
		case '\t':
			b.WriteString(`\t`)
		default:
			if r < 0x20 {
				b.WriteString(fmt.Sprintf("\\u%04X", r))
			} else {
				b.WriteRune(r)
			}
		}
		i += size
	}
	return b.String()
}

func parseNumber(s string, allowHex bool, allowSpecial bool) (float64, bool) {
	if allowHex {
		sign := 1.0
		hex := s
		if strings.HasPrefix(hex, "-") {
			sign = -1
			hex = hex[1:]
		}
		if strings.HasPrefix(hex, "0x") || strings.HasPrefix(hex, "0X") {
			if v, err := strconv.ParseInt(hex[2:], 16, 64); err == nil {
				return sign * float64(v), true
			}
		}
	}

	if allowSpecial {
		sign := 1.0
		val := s
		if strings.HasPrefix(val, "-") {
			sign = -1
			val = val[1:]
		}
		if strings.EqualFold(val, "Infinity") {
			return sign * math.Inf(1), true
		}
		if strings.EqualFold(val, "NaN") {
			return math.NaN(), true
		}
	}

	if v, err := strconv.ParseFloat(s, 64); err == nil {
		return v, true
	}
	return 0, false
}

func formatNumber(v float64, allowNaN bool) string {
	if math.IsNaN(v) {
		if allowNaN {
			return "NaN"
		}
		return "null"
	}
	if math.IsInf(v, 1) {
		if allowNaN {
			return "Infinity"
		}
		return "null"
	}
	if math.IsInf(v, -1) {
		if allowNaN {
			return "-Infinity"
		}
		return "null"
	}
	return strconv.FormatFloat(v, 'g', -1, 64)
}
