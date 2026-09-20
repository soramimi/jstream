using System;
using System.Collections.Generic;
using System.Globalization;
using System.Text;
using System.Text.RegularExpressions;

namespace JStream;

public class Error {
	public string Message { get; }
	public int Offset { get; }
	public int Line { get; }
	public int Column { get; }

	public Error(string message, int offset, int line, int column)
	{
		Message = message;
		Offset = offset;
		Line = line;
		Column = column;
	}

	public override string ToString() => Message;
}

public class Reader {
	private readonly string _json;
	private int _position;
	private readonly List<StateItem> _states = new();
	private readonly List<string> _depth = new();
	private readonly List<NestItem> _depthStack = new();
	private string _key = string.Empty;
	private string _stringValue = string.Empty;
	private double _numberValue;
	private bool _isArray;
	private bool _hold;
	private StateItem? _lastState;
	private readonly List<Error> _errors = new();

	public bool AllowComment { get; set; }
	public bool AllowAmbiguousComma { get; set; }
	public bool AllowUnquotedKey { get; set; }
	public bool AllowHexadecimal { get; set; }
	public bool AllowSpecialConstant { get; set; }
	public bool AllowKeyInArray { get; set; }

	private struct StateItem {
		public StateType Type { get; set; }
		public int Position { get; set; }

		public StateItem(StateType type, int position = 0)
		{
			Type = type;
			Position = position;
		}
	}

	private struct NestItem {
		public int Depth { get; set; }
		public string Path { get; set; }
	}

	public Reader(string json)
	{
		_json = json ?? throw new ArgumentNullException(nameof(json));
		_position = 0;
	}

	public StateType State => _states.Count > 0 ? _states[^1].Type : StateType.None;
	public bool HasError => _errors.Count > 0;
	public IReadOnlyList<Error> Errors => _errors;
	public bool IsStartObject => State == StateType.StartObject;
	public bool IsEndObject => State == StateType.EndObject;
	public bool IsStartArray => State == StateType.StartArray;
	public bool IsEndArray => State == StateType.EndArray;
	public bool IsConstant => IsConstantState();
	public bool IsStructure => IsStructureState();
	public bool IsValue => IsConstant || IsStructure;
	public string Key => _key;
	public string StringValue => _stringValue;
	public StateType Symbol => GetSymbol();
	public bool IsNull => Symbol == StateType.Null;
	public bool IsFalse => Symbol == StateType.False;
	public bool IsTrue => Symbol == StateType.True;
	public bool IsBoolean => IsFalse || IsTrue;
	public bool IsNumber => State == StateType.Number;
	public bool IsString => State == StateType.String;
	public double Number => _numberValue;
	public bool BooleanValue => IsTrue;
	public bool IsArray => _isArray;
	public int Depth => _depth.Count;
	public string Path => GetPath();
	public int Tell => _position;

	private bool IsStructureState()
	{
		return State switch {
			StateType.StartObject or StateType.StartArray => true,
			StateType.EndObject or StateType.EndArray when _states.Count > 1 =>
				_states[^2].Type is StateType.StartObject or StateType.StartArray,
			_ => false
		};
	}

	private bool IsConstantState()
	{
		return State is StateType.String or StateType.Number or StateType.Null or StateType.False or StateType.True;
	}

	private StateType GetSymbol()
	{
		return State switch {
			StateType.Null or StateType.False or StateType.True => State,
			_ => StateType.None
		};
	}

	private string GetPath()
	{
		var path = new StringBuilder();
		foreach (var s in _depth) {
			path.Append(s);
		}

		if (State is StateType.StartObject or StateType.StartArray)
			return path.ToString();

		return path.ToString() + _key;
	}

	public void Reset()
	{
		_errors.Clear();
	}

	public void Hold()
	{
		_hold = true;
	}

    public void Nest()
    {
        _depthStack.Add(new NestItem { Depth = Depth, Path = Path });
    }

    public void Nest(Action callback)
    {
        Nest();
        do {
            callback();
        } while (Next());
    }

    public string Extract()
	{
		if (_lastState != null) {
			int pos = _lastState.Value.Position;
			return _json.Substring(pos, _position - pos);
		}
		return string.Empty;
	}

	public ReadOnlySpan<char> Extract(int begin, int end)
	{
		if (begin >= 0 && end <= _json.Length && begin <= end) {
			return _json.AsSpan(begin, end - begin);
		}
		return ReadOnlySpan<char>.Empty;
	}

	public bool Next()
	{
		if (_hold) {
			_hold = false;
			return true;
		}

        if (InternalNext()) {
            if (_depthStack.Count == 0)
                return true;

            if (Depth >= _depthStack[^1].Depth)
                return true;

            _depthStack.RemoveAt(_depthStack.Count - 1);
            Hold();
        }

		return false;
	}

	private bool InternalNext()
	{
		while (_position < _json.Length) {
			SkipWhitespaceAndComments();

			if (_position >= _json.Length)
				break;

			char ch = _json[_position];

			switch (ch) {
			case '}':
				return HandleEndObject();
			case ']':
				return HandleEndArray();
			case ',':
				return HandleComma();
			case '{':
				return HandleStartObject();
			case '[':
				return HandleStartArray();
			case '"':
				return HandleString();
			default:
				if (State == StateType.Key || IsArray) {
					if (char.IsDigit(ch) || ch == '-' || ch == '+' || ch == '.')
						return HandleNumber();

					if (char.IsLetter(ch))
						return HandleSymbol();
				} else if (AllowUnquotedKey && char.IsLetter(ch)) {
					return HandleUnquotedKey();
				}

				PushError("Syntax error");
				return false;
			}
		}

		return false;
	}

	private void SkipWhitespaceAndComments()
	{
		while (_position < _json.Length) {
			char ch = _json[_position];

			if (char.IsWhiteSpace(ch)) {
				_position++;
				continue;
			}

			if (AllowComment && ch == '/' && _position + 1 < _json.Length) {
				char nextCh = _json[_position + 1];
				if (nextCh == '/') {
					_position += 2;
					while (_position < _json.Length && _json[_position] != '\r' && _json[_position] != '\n')
						_position++;
					continue;
				}

				if (nextCh == '*') {
					_position += 2;
					while (_position + 1 < _json.Length) {
						if (_json[_position] == '*' && _json[_position + 1] == '/') {
							_position += 2;
							break;
						}
						_position++;
					}
					continue;
				}
			}

			break;
		}
	}

	private bool HandleEndObject()
	{
		_position++;
		_stringValue = string.Empty;
		string key = string.Empty;

		if (_depth.Count > 0) {
			key = _depth[^1];
			if (key.EndsWith('{'))
				key = key[..^1];
			_depth.RemoveAt(_depth.Count - 1);
		}

		while (true) {
			bool wasStartObject = State == StateType.StartObject;
			if (!PopState())
				break;

			if (wasStartObject) {
				PushState(new StateItem(StateType.EndObject));
				_key = key;
				return true;
			}
		}

		return false;
	}

	private bool HandleEndArray()
	{
		_position++;
		_stringValue = string.Empty;
		string key = string.Empty;

		if (_depth.Count > 0) {
			key = _depth[^1];
			if (key.EndsWith('['))
				key = key[..^1];
			_depth.RemoveAt(_depth.Count - 1);
		}

		while (true) {
			bool wasStartArray = State == StateType.StartArray;
			if (!PopState())
				break;

			if (wasStartArray) {
				PushState(new StateItem(StateType.EndArray));
				_key = key;
				return true;
			}
		}

		return false;
	}

	private bool HandleComma()
	{
		_position++;

		if (State == StateType.Key) {
			PushState(new StateItem(StateType.Null));
			return true;
		}

		SkipWhitespaceAndComments();

		if (IsStructure || IsValue)
			PopState();

		PushState(new StateItem(StateType.Comma));

		if (AllowAmbiguousComma)
			return Next();

		return true;
	}

	private bool HandleStartObject()
	{
		int pos = _position++;

		if (State != StateType.Key) {
			_key = string.Empty;
			_stringValue = string.Empty;
		}

		_depth.Add(_key + "{");
		PushState(new StateItem(StateType.StartObject, pos));
		return true;
	}

	private bool HandleStartArray()
	{
		int pos = _position++;

		if (State != StateType.Key) {
			_key = string.Empty;
			_stringValue = string.Empty;
		}

		_depth.Add(_key + "[");
		PushState(new StateItem(StateType.StartArray, pos));
		return true;
	}

	private bool HandleString()
	{
		var result = ParseString();
		if (!result.Success) {
			PushError("Invalid string");
			return false;
		}

		_stringValue = result.Value;
		_position = result.EndPosition;

		if (State == StateType.Key) {
			PushState(new StateItem(StateType.String));
			return true;
		}

		SkipWhitespaceAndComments();

		if (_position < _json.Length && _json[_position] == ':') {
			if (IsArray) {
				if (!AllowKeyInArray) {
					PushError("Unexpected key in array");
					return false;
				}
			}
			_position++;
			_key = _stringValue;
			PushState(new StateItem(StateType.Key));
			return true;
		}

		PushState(new StateItem(StateType.String));
		return true;
	}

	private bool HandleNumber()
	{
		var result = ParseNumber();
		if (!result.Success)
			return false;

		_stringValue = result.Text;
		_numberValue = result.Value;
		_position = result.EndPosition;

		PushState(new StateItem(StateType.Number));
		return true;
	}

	private bool HandleSymbol()
	{
		var result = ParseSymbol();
		if (!result.Success)
			return false;

		_stringValue = result.Value;
		_position = result.EndPosition;

		var stateType = result.Value switch {
			"false" => StateType.False,
			"true" => StateType.True,
			"null" => StateType.Null,
			_ => StateType.None
		};

		if (stateType != StateType.None) {
			PushState(new StateItem(stateType));
			return true;
		}

		return false;
	}

	private bool HandleUnquotedKey()
	{
		var result = ParseSymbol();
		if (!result.Success)
			return false;

		_stringValue = result.Value;
		int endPos = result.EndPosition;

		int tempPos = endPos;
		while (tempPos < _json.Length && char.IsWhiteSpace(_json[tempPos]))
			tempPos++;

		if (tempPos < _json.Length && _json[tempPos] == ':') {
			_position = tempPos + 1;
			_key = _stringValue;
			PushState(new StateItem(StateType.Key));
			return true;
		}

		return false;
	}

	private (bool Success, string Value, int EndPosition) ParseSymbol()
	{
		int start = _position;
		while (_position < _json.Length && (char.IsLetterOrDigit(_json[_position]) || _json[_position] == '_'))
			_position++;

		if (_position > start)
			return (true, _json[start.._position], _position);

		return (false, string.Empty, start);
	}

	private (bool Success, string Value, int EndPosition) ParseString()
	{
		if (_position >= _json.Length || _json[_position] != '"')
			return (false, string.Empty, _position);

		int start = ++_position;
		var sb = new StringBuilder();

		while (_position < _json.Length) {
			char ch = _json[_position];

			if (ch == '"') {
				_position++;
				return (true, sb.ToString(), _position);
			}

			if (ch == '\\') {
				_position++;
				if (_position >= _json.Length)
					break;

				char escapedChar = _json[_position];
				switch (escapedChar) {
				case 'b':
					sb.Append('\b');
					break;
				case 'n':
					sb.Append('\n');
					break;
				case 'r':
					sb.Append('\r');
					break;
				case 'f':
					sb.Append('\f');
					break;
				case 't':
					sb.Append('\t');
					break;
				case 'v':
					sb.Append('\v');
					break;
				case '\\':
				case '"':
				case '/':
					sb.Append(escapedChar);
					break;
				case 'u':
				{
					if (_position + 4 > _json.Length)
						return (false, string.Empty, start);

					string hexString = _json.Substring(_position + 1, 4);
					if (!int.TryParse(hexString, NumberStyles.HexNumber, null, out int codeUnit))
						return (false, string.Empty, start);

					_position += 4;

					if (codeUnit >= 0xD800 && codeUnit < 0xDC00) {
						if (_position + 5 < _json.Length && _json[_position] == '\\' && _json[_position + 1] == 'u') {
							string lowHex = _json.Substring(_position + 2, 4);
							if (int.TryParse(lowHex, NumberStyles.HexNumber, null, out int lowSurrogate)
								&& lowSurrogate >= 0xDC00 && lowSurrogate < 0xE000) {
								_position += 6;
								int unicode = ((codeUnit - 0xD800) << 10) + (lowSurrogate - 0xDC00) + 0x10000;
								sb.Append(char.ConvertFromUtf32(unicode));
							} else {
								return (false, string.Empty, start);
							}
						} else {
							return (false, string.Empty, start);
						}
					} else if (codeUnit >= 0xDC00 && codeUnit < 0xE000) {
						return (false, string.Empty, start);
					} else {
						sb.Append(char.ConvertFromUtf32(codeUnit));
					}
					break;
				}
				default:
					sb.Append(escapedChar);
					break;
				}
				_position++;
			} else {
				sb.Append(ch);
				_position++;
			}
		}

		return (false, string.Empty, start);
	}

	private (bool Success, double Value, string Text, int EndPosition) ParseNumber()
	{
		int start = _position;

		while (_position < _json.Length) {
			char ch = _json[_position];
			if (char.IsDigit(ch) || ch == '.' || ch == '+' || ch == '-' || ch == 'e' || ch == 'E')
				_position++;
			else
				break;
		}

		if (_position > start) {
			string numberText = _json[start.._position];
			double value = JsonHelper.ParseNumber(numberText, AllowHexadecimal, AllowSpecialConstant);
			return (true, value, numberText, _position);
		}

		return (false, 0.0, string.Empty, start);
	}

	private void PushState(StateItem state)
	{
		if (State is StateType.Key or StateType.Comma or StateType.EndObject)
			_states.RemoveAt(_states.Count - 1);

		_states.Add(state);

		if (IsArray)
			_key = string.Empty;

		switch (state.Type) {
		case StateType.StartArray:
			_isArray = true;
			break;
		case StateType.StartObject:
		case StateType.Key:
			_isArray = false;
			break;
		}
	}

	private bool PopState()
	{
		if (_states.Count == 0)
			return false;

		_lastState = _states[^1];
		_states.RemoveAt(_states.Count - 1);

		int i = _states.Count;
		while (i > 0) {
			i--;
			var state = _states[i];
			if (state.Type == StateType.StartArray) {
				_isArray = true;
				break;
			}
			if (state.Type is StateType.StartObject or StateType.Key) {
				_isArray = false;
				break;
			}
		}

		if (State == StateType.Key)
			_states.RemoveAt(_states.Count - 1);

		_key = string.Empty;
		return true;
	}

	private void PushError(string message)
	{
		_states.Clear();
		int offset = _position;
		int line = 1;
		int column = 1;
		for (int i = 0; i < _position && i < _json.Length; i++) {
			if (_json[i] == '\n') {
				line++;
				column = 1;
			} else if (_json[i] != '\r') {
				column++;
			}
		}
		_errors.Add(new Error(message, offset, line, column));
	}

    public bool Match(string path, bool matchEndStructure = false)
    {
        if (!IsValue)
            return false;

        if (!string.IsNullOrEmpty(path) && path[0] == '@') {
            if (_depthStack.Count > 0) {
                var item = _depthStack[^1];
                path = item.Path + path.Substring(1);
            }
        }

        int pathPos = 0;

		char Path(int i) => i < path.Length ? path[i] : '\0';

		for (int i = 0; i < _depth.Count; i++) {
			string element = _depth[i];
			if (string.IsNullOrEmpty(element))
				return false;

			if (Path(pathPos) == '*') {
				if (Path(pathPos + 1) == '*') {
					if (Path(pathPos + 2) == '\0')
						return true;
					return false; // "**" must be at the end of path
				}

				char c = element[^1];
				if (c == '{' || c == '[') {
					if (Path(pathPos + 1) == c) {
						pathPos += 2;
						continue;
					}
					if (Path(pathPos + 1) == '\0') {
						if (i + 1 == _depth.Count) {
							if (c == '{' && State == StateType.StartObject)
								return true;
							if (c == '[' && State == StateType.StartArray)
								return true;
						}
						return false;
					}
				}
			}

			if (pathPos + element.Length > path.Length)
				return false;
			if (path.Substring(pathPos, element.Length) != element)
				return false;
			pathPos += element.Length;
		}

		if (Path(pathPos) == '*') {
			if (Path(pathPos + 1) == '*' && Path(pathPos + 2) == '\0')
				return true;
			if (Path(pathPos + 1) == '\0') {
				if (IsConstant)
					return true;
				if (matchEndStructure && (State == StateType.EndObject || State == StateType.EndArray))
					return true;
				return false;
			}
		}

		string remaining = pathPos < path.Length ? path[pathPos..] : string.Empty;
		return remaining == _key;
	}

	public bool MatchStartObject(string path) => State == StateType.StartObject && Match(path);
	public bool MatchEndObject(string path) => State == StateType.EndObject && Match(path, true);
	public bool MatchStartArray(string path) => State == StateType.StartArray && Match(path);
	public bool MatchEndArray(string path) => State == StateType.EndArray && Match(path, true);

	public Variant GetVariant()
	{
		if (IsNull)
			return Variant.Null;
		if (IsFalse)
			return new Variant(false);
		if (IsTrue)
			return new Variant(true);
		if (IsNumber)
			return new Variant(Number);
		if (IsString)
			return new Variant(StringValue);
		return Variant.Null;
	}
}