using System;
using System.Collections.Generic;
using Xunit;

namespace JStream.Tests;

public class BasicTests
{
    [Fact]
    public void Variant_BasicTypes_Test()
    {
        Variant v = Variant.Null;
        Assert.True(v.IsNull);
        
        v = new Variant(true);
        Assert.Equal(true, v.Get<bool>());
        
        v = new Variant(false);
        Assert.Equal(false, v.Get<bool>());
        
        v = new Variant(3.14);
        Assert.Equal(3.14, v.Get<double>());
        
        v = new Variant("Hello");
        Assert.Equal("Hello", v.Get<string>());
    }

    [Fact]
    public void Reader_SimpleObject_Test()
    {
        const string json = """
        {
            "name": "John",
            "age": 30,
            "city": "New York"
        }
        """;
        
        var reader = new Reader(json);
        string? name = null;
        double age = 0;
        string? city = null;
        
        while (reader.Next())
        {
            if (reader.Match("{name"))
            {
                name = reader.StringValue;
            }
            else if (reader.Match("{age"))
            {
                age = reader.Number;
            }
            else if (reader.Match("{city"))
            {
                city = reader.StringValue;
            }
        }
        
        Assert.Equal("John", name);
        Assert.Equal(30, age);
        Assert.Equal("New York", city);
    }

    [Fact]
    public void Reader_NestedObject_Test()
    {
        const string json = """
        {
            "name": "John",
            "age": 30,
            "city": "New York",
            "address": {
                "street": "123 Main St",
                "zip": "10001"
            }
        }
        """;
        
        var reader = new Reader(json);
        string? name = null;
        double age = 0;
        string? city = null;
        string? street = null;
        string? zip = null;
        
        while (reader.Next())
        {
            if (reader.Match("{name"))
            {
                name = reader.StringValue;
            }
            else if (reader.Match("{age"))
            {
                age = reader.Number;
            }
            else if (reader.Match("{city"))
            {
                city = reader.StringValue;
            }
            else if (reader.Match("{address{street"))
            {
                street = reader.StringValue;
            }
            else if (reader.Match("{address{zip"))
            {
                zip = reader.StringValue;
            }
        }
        
        Assert.Equal("John", name);
        Assert.Equal(30, age);
        Assert.Equal("New York", city);
        Assert.Equal("123 Main St", street);
        Assert.Equal("10001", zip);
    }

    [Fact]
    public void Reader_Array_Test()
    {
        const string json = """
        {
            "a": [ 12, 34, 56, 78 ]
        }
        """;
        
        var numbers = new List<double>();
        var reader = new Reader(json);
        
        while (reader.Next())
        {
            if (reader.Match("{a[*"))
            {
                numbers.Add(reader.Number);
            }
        }
        
        Assert.Equal(4, numbers.Count);
        Assert.Equal(12, numbers[0]);
        Assert.Equal(34, numbers[1]);
        Assert.Equal(56, numbers[2]);
        Assert.Equal(78, numbers[3]);
    }

    [Fact]
    public void Writer_SimpleObject_Test()
    {
        var output = new List<string>();
        var writer = new Writer(s => output.Add(s));
        
        writer.Object("", () =>
        {
            writer.String("name", "John Doe");
            writer.Number("age", 30);
            writer.String("city", "New York");
        });
        
        string result = string.Join("", output);
        
        // Parse it back to verify
        var reader = new Reader(result);
        string? name = null;
        double age = 0;
        string? city = null;
        
        while (reader.Next())
        {
            if (reader.Match("{name"))
                name = reader.StringValue;
            else if (reader.Match("{age"))
                age = reader.Number;
            else if (reader.Match("{city"))
                city = reader.StringValue;
        }
        
        Assert.Equal("John Doe", name);
        Assert.Equal(30, age);
        Assert.Equal("New York", city);
    }

    [Fact]
    public void Reader_ObjectArray_Test()
    {
        const string json = """
        [
            { "name": "apple", "price": 150 },
            { "name": "banana", "price": 80 },
            { "name": "orange", "price": 52 }
        ]
        """;
        
        var items = new List<(string name, double price)>();
        var reader = new Reader(json);
        
        while (reader.Next())
        {
            if (reader.MatchStartObject("[**"))
            {
                string? name = null;
                double price = 0;
                
                reader.Nest();
                do
                {
                    if (reader.Match("[{name"))
                        name = reader.StringValue;
                    else if (reader.Match("[{price"))
                        price = reader.Number;
                } while (reader.Next());
                
                if (name != null)
                    items.Add((name, price));
            }
        }
        
        Assert.Equal(3, items.Count);
        Assert.Equal("apple", items[0].name);
        Assert.Equal(150, items[0].price);
        Assert.Equal("banana", items[1].name);
        Assert.Equal(80, items[1].price);
        Assert.Equal("orange", items[2].name);
        Assert.Equal(52, items[2].price);
    }
}