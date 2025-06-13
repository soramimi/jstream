using System;
using System.Collections.Generic;

namespace JStream.Tests;

public static class TestHelper
{
    public static string StateToString(StateType state)
    {
        return state switch
        {
            StateType.None => "None",
            StateType.Null => "Null",
            StateType.False => "False",
            StateType.True => "True",
            StateType.Key => "Key",
            StateType.Comma => "Comma",
            StateType.StartObject => "StartObject",
            StateType.EndObject => "EndObject",
            StateType.StartArray => "StartArray",
            StateType.EndArray => "EndArray",
            StateType.String => "String",
            StateType.Number => "Number",
            _ => "Invalid"
        };
    }

    public static List<TestEvent> ParseToEvents(string json)
    {
        var events = new List<TestEvent>();
        var reader = new Reader(json);
        
        while (reader.Next())
        {
            var eventItem = new TestEvent
            {
                State = reader.State,
                Path = reader.Path
            };
            
            switch (reader.State)
            {
                case StateType.Key:
                case StateType.StartObject:
                case StateType.EndObject:
                case StateType.StartArray:
                case StateType.EndArray:
                    eventItem.Key = reader.Key;
                    break;
                default:
                    if (reader.IsValue)
                    {
                        eventItem.Value = reader.StringValue;
                    }
                    break;
            }
            
            events.Add(eventItem);
        }
        
        return events;
    }

    public static void PrintEvents(List<TestEvent> events)
    {
        foreach (var evt in events)
        {
            string stateType = StateToString(evt.State);
            Console.WriteLine($"{{{stateType}, \"{evt.Key}\", \"{evt.Value}\", \"{evt.Path}\"}},");
        }
    }

    public static void PrintEvents(string json)
    {
        var events = ParseToEvents(json);
        PrintEvents(events);
    }

    public static void TestParseEvent(string json, List<TestEvent> expected)
    {
        var actual = ParseToEvents(json);
        
        if (actual.Count != expected.Count)
        {
            throw new Exception($"Event count mismatch. Expected: {expected.Count}, Actual: {actual.Count}");
        }
        
        for (int i = 0; i < actual.Count; i++)
        {
            if (!actual[i].Equals(expected[i]))
            {
                throw new Exception($"Event mismatch at index {i}. Expected: {expected[i]}, Actual: {actual[i]}");
            }
        }
    }
}