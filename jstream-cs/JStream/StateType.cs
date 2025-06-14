namespace JStream;

public enum StateType {
	None = 0,
	Null,
	False,
	True,
	Key = 100,
	Comma,
	StartObject,
	EndObject,
	StartArray,
	EndArray,
	String,
	Number,
}