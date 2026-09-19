package main

import (
	"fmt"
	"time"

	"github.com/soramimi/jstream/jstream-go"
)

func main() {
	json := `{
		"book": [
			{"id": "444", "language": "C", "edition": "First", "author": "Dennis Ritchie"},
			{"id": "555", "language": "C++", "edition": "Second", "author": "Bjarne Stroustrup"}
		]
	}`

	iterations := 100000
	fmt.Printf("Parsing JSON %d times...\n", iterations)
	start := time.Now()
	for i := 0; i < iterations; i++ {
		reader := jstream.NewReader(json)
		for reader.Next() {
			if reader.MatchStartObject("{book[*{") {
				reader.Nest(func() {
					_ = reader.Key()
				})
			}
		}
	}
	fmt.Printf("Parsed JSON in %d ms\n", time.Since(start).Milliseconds())
}
