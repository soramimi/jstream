using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;

namespace JStream;

public class Reader {
	private readonly string _json;
	private int _position;
	private readonly List<StateItem> _states = new();
	private readonly List<string> _depth = new();
	private readonly List<int> _depthStack = new();
	private string _key = string.Empty;
	private string _stringValue = string.Empty;
	private double _numberValue;
	private bool _isArray;
	private bool _hold;
	private StateItem? _lastState;
	private readonly List<string> _errors = new();

	public bool AllowComment { get; set; }
	public bool AllowAmbiguousComma { get; set; }
	public bool AllowUnquotedKey { get; set; }
	public bool AllowHexadecimal { get; set; }
	public bool AllowSpecialConstant { get; set; }

	private struct StateItem {
		public StateType Type { get; set; }
		public int Position { get; set; }

		public StateItem(StateType type, int position = 0)
		{
			Type = type;
			Position = position;
		}
	}

	public Reader(string json)
	{
		_json = json ?? throw new ArgumentNullException(nameof(json));
		_position = 0;
	}

	public StateType State => _states.Count > 0 ? _states[^1].Type : StateType.None;
	public bool HasError => _errors.Count > 0;
	public IReadOnlyList<string> Errors => _errors;
	public bool IsStartObject => State == StateType.StartObject;
	public bool IsEndObject => State == StateType.EndObject;
	public bool IsStartArray => State == StateType.StartArray;
	public bool IsEndArray => State == StateType.EndArray;
	public bool IsObject => IsObjectOrArray();
	public bool IsValue => IsValueState();
	public string Key => _key;
	public string StringValue => _stringValue;
	public StateType Symbol => GetSymbol();
	public bool IsNull => Symbol == StateType.Null;
	public bool IsFalse => Symbol == StateType.False;
	public bool IsTrue => Symbol == StateType.True;
	public bool IsNumber => State == StateType.Number;
	public bool IsString => State == StateType.String;
	public double Number => _numberValue;
	public bool IsArray => _isArray;
	public int Depth => _depth.Count;
	public string Path => GetPath();

	private bool IsObjectOrArray()
	{
		return State switch {
			StateType.StartObject or StateType.StartArray => true,
			StateType.EndObject or StateType.EndArray when _states.Count > 1 =>
				_states[^2].Type is StateType.StartObject or StateType.StartArray,
			_ => false
		};
	}

	private bool IsValueState()
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
		_depthStack.Add(Depth);
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

			if (Depth >= _depthStack[^1])
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

		if (IsObject || IsValue)
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

		if (IsArray) {
			PushState(new StateItem(StateType.String));
			return true;
		}

		if (State == StateType.Key) {
			PushState(new StateItem(StateType.String));
			return true;
		}

		SkipWhitespaceAndComments();

		if (_position < _json.Length && _json[_position] == ':') {
			_position++;
			_key = _stringValue;
			PushState(new StateItem(StateType.Key));
			return true;
		}

		return false;
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
					if (_position + 4 < _json.Length) {
						string hexString = _json.Substring(_position + 1, 4);
						if (int.TryParse(hexString, System.Globalization.NumberStyles.HexNumber, null, out int unicode)) {
							sb.Append((char)unicode);
							_position += 4;
						} else {
							sb.Append(escapedChar);
						}
					} else {
						sb.Append(escapedChar);
					}
					break;
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

	private void PushError(string error)
	{
		_states.Clear();
		_errors.Add(error);
	}

	public bool Match(string path)
	{
		if (!IsObject && !IsArray && !IsValue)
			return false;

		// C++版に基づく実装
		int pathPos = 0;

		for (int i = 0; i < _depth.Count; i++) {
			string depthItem = _depth[i];
			if (string.IsNullOrEmpty(depthItem))
				break;

			// ** ワイルドカード - 任意の深さまでマッチ
			if (pathPos < path.Length - 1 && path[pathPos] == '*' && path[pathPos + 1] == '*' &&
				(pathPos + 2 >= path.Length || path[pathPos + 2] == '\0')) {
				return true;
			}

			// * ワイルドカード処理
			if (pathPos < path.Length && path[pathPos] == '*' && depthItem.EndsWith('{')) {
				if (pathPos + 1 >= path.Length) {
					return (State == StateType.StartObject && i + 1 == _depth.Count);
				} else if (pathPos + 1 < path.Length && path[pathPos + 1] == '{') {
					pathPos += 2;
					continue;
				}
			}

			// 通常のパスマッチング
			if (pathPos + depthItem.Length <= path.Length &&
				path.Substring(pathPos, depthItem.Length) == depthItem) {
				pathPos += depthItem.Length;
			} else {
				return false;
			}
		}

		// 残りのパスの処理
		if (pathPos < path.Length - 1 && path[pathPos] == '*' && path[pathPos + 1] == '*' &&
			(pathPos + 2 >= path.Length || path[pathPos + 2] == '\0')) {
			return true;
		}

		if (pathPos < path.Length && path[pathPos] == '*') {
			return (pathPos + 1 >= path.Length && _depth.Count > 0
				&& (IsValue || State == StateType.EndObject || State == StateType.EndArray));
		}

		string remainingPath = pathPos < path.Length ? path[pathPos..] : string.Empty;
		return remainingPath == _key;
	}

	public bool MatchStartObject(string path) => State == StateType.StartObject && Match(path);
	public bool MatchEndObject(string path) => State == StateType.EndObject && Match(path);
	public bool MatchStartArray(string path) => State == StateType.StartArray && Match(path);
	public bool MatchEndArray(string path) => State == StateType.EndArray && Match(path);

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