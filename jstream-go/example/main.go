package main

import (
	"fmt"

	"github.com/soramimi/jstream/jstream-go"
)

func main() {
	json := `{
		"name": "John Doe",
		"age": 30,
		"cities": ["New York", "London", "Tokyo"]
	}`

	reader := jstream.NewReader(json)
	for reader.Next() {
		if reader.Match("{name") && reader.IsString() {
			fmt.Printf("Name: %s\n", reader.StringValue())
		} else if reader.Match("{age") && reader.IsNumber() {
			fmt.Printf("Age: %v\n", reader.Number())
		} else if reader.Match("{cities[*") && reader.IsString() {
			fmt.Printf("City: %s\n", reader.StringValue())
		}
	}

	writer := jstream.NewWriter()
	writer.Object("", func() {
		writer.WriteString("name", "John Doe")
		writer.WriteNumber("age", 30)
		writer.Array("cities", func() {
			writer.WriteStringValue("New York")
			writer.WriteStringValue("London")
			writer.WriteStringValue("Tokyo")
		})
		writer.WriteBoolean("active", true)
		writer.WriteNull("optionalField")
	})
	fmt.Println(writer.String())

	root := jstream.NewVariant()
	obj := root.AsObject()
	obj.Set("name", jstream.NewString("John Doe"))
	obj.Set("age", jstream.NewNumber(30))
	citiesVariant := obj.Get("cities")
	cities := citiesVariant.AsArray()
	cities.Append(jstream.NewString("New York"))
	cities.Append(jstream.NewString("London"))
	cities.Append(jstream.NewString("Tokyo"))
	if v, ok := obj.Get("name").TryString(); ok {
		fmt.Printf("Variant Name: %s\n", v)
	}
}
