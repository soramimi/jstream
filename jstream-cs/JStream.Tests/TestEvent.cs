using System;

namespace JStream.Tests;

public struct TestEvent {
	public StateType State { get; set; }
	public string Key { get; set; }
	public string Value { get; set; }
	public string Path { get; set; }

	public TestEvent(StateType state, string key, string value, string path)
	{
		State = state;
		Key = key ?? string.Empty;
		Value = value ?? string.Empty;
		Path = path ?? string.Empty;
	}

	public override bool Equals(object? obj)
	{
		if (obj is not TestEvent other)
			return false;

		return State == other.State &&
			   Key == other.Key &&
			   Value == other.Value &&
			   Path == other.Path;
	}

	public override int GetHashCode()
	{
		return HashCode.Combine(State, Key, Value, Path);
	}

	public override string ToString()
	{
		return $"{{State={State}, Key=\"{Key}\", Value=\"{Value}\", Path=\"{Path}\"}}";
	}
}