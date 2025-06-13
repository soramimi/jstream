using System;
using System.Collections;
using System.Collections.Generic;

namespace JStream;

public class Variant
{
    private readonly object? _value;

    public Variant() => _value = null;
    public Variant(bool value) => _value = value;
    public Variant(double value) => _value = value;
    public Variant(string value) => _value = value;
    public Variant(JObject value) => _value = value;
    public Variant(JArray value) => _value = value;
    private Variant(object? value) => _value = value;

    public bool IsNull => _value is null;
    public bool IsBoolean => _value is bool;
    public bool IsNumber => _value is double;
    public bool IsString => _value is string;
    public bool IsObject => _value is JObject;
    public bool IsArray => _value is JArray;
    public bool IsNaN => _value is double d && double.IsNaN(d);
    public bool IsInfinite => _value is double d && double.IsInfinity(d);

    public T Get<T>()
    {
        if (_value is T value)
            return value;
        
        if (_value is null && !typeof(T).IsValueType)
            return default(T)!;
            
        throw new InvalidOperationException($"Cannot convert {_value?.GetType().Name ?? "null"} to {typeof(T).Name}");
    }

    public bool TryGet<T>(out T value)
    {
        if (_value is T val)
        {
            value = val;
            return true;
        }
        
        if (_value is null && !typeof(T).IsValueType)
        {
            value = default(T)!;
            return true;
        }
        
        value = default(T)!;
        return false;
    }

    public static implicit operator Variant(bool value) => new(value);
    public static implicit operator Variant(double value) => new(value);
    public static implicit operator Variant(string value) => new(value);
    public static implicit operator Variant(JObject value) => new(value);
    public static implicit operator Variant(JArray value) => new(value);

    public static readonly Variant Null = new();
}

public class JObject : IDictionary<string, Variant>
{
    private readonly Dictionary<string, Variant> _dictionary = new();

    public Variant this[string key]
    {
        get => _dictionary.TryGetValue(key, out var value) ? value : Variant.Null;
        set => _dictionary[key] = value;
    }

    public ICollection<string> Keys => _dictionary.Keys;
    public ICollection<Variant> Values => _dictionary.Values;
    public int Count => _dictionary.Count;
    public bool IsReadOnly => false;

    public void Add(string key, Variant value) => _dictionary.Add(key, value);
    public void Add(KeyValuePair<string, Variant> item) => _dictionary.Add(item.Key, item.Value);
    public void Clear() => _dictionary.Clear();
    public bool Contains(KeyValuePair<string, Variant> item) => ((IDictionary<string, Variant>)_dictionary).Contains(item);
    public bool ContainsKey(string key) => _dictionary.ContainsKey(key);
    public void CopyTo(KeyValuePair<string, Variant>[] array, int arrayIndex) => ((IDictionary<string, Variant>)_dictionary).CopyTo(array, arrayIndex);
    public IEnumerator<KeyValuePair<string, Variant>> GetEnumerator() => _dictionary.GetEnumerator();
    public bool Remove(string key) => _dictionary.Remove(key);
    public bool Remove(KeyValuePair<string, Variant> item) => ((IDictionary<string, Variant>)_dictionary).Remove(item);
    public bool TryGetValue(string key, out Variant value) => _dictionary.TryGetValue(key, out value!);
    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

    public T Get<T>(string key) => this[key].Get<T>();
    public bool TryGet<T>(string key, out T value)
    {
        if (_dictionary.TryGetValue(key, out var variant))
            return variant.TryGet(out value);
        
        value = default(T)!;
        return false;
    }
}

public class JArray : IList<Variant>
{
    private readonly List<Variant> _list = new();

    public Variant this[int index]
    {
        get => _list[index];
        set => _list[index] = value;
    }

    public int Count => _list.Count;
    public bool IsReadOnly => false;

    public void Add(Variant item) => _list.Add(item);
    public void Clear() => _list.Clear();
    public bool Contains(Variant item) => _list.Contains(item);
    public void CopyTo(Variant[] array, int arrayIndex) => _list.CopyTo(array, arrayIndex);
    public IEnumerator<Variant> GetEnumerator() => _list.GetEnumerator();
    public int IndexOf(Variant item) => _list.IndexOf(item);
    public void Insert(int index, Variant item) => _list.Insert(index, item);
    public bool Remove(Variant item) => _list.Remove(item);
    public void RemoveAt(int index) => _list.RemoveAt(index);
    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

    public T Get<T>(int index) => this[index].Get<T>();
    public bool TryGet<T>(int index, out T value)
    {
        if (index >= 0 && index < _list.Count)
            return _list[index].TryGet(out value);
        
        value = default(T)!;
        return false;
    }

    public static JArray operator +(JArray array, Variant value)
    {
        array.Add(value);
        return array;
    }
}

public static class VariantExtensions
{
    public static JObject AsObject(this Variant variant)
    {
        if (variant.IsObject)
            return variant.Get<JObject>();
        
        var obj = new JObject();
        variant = obj;
        return obj;
    }

    public static JArray AsArray(this Variant variant)
    {
        if (variant.IsArray)
            return variant.Get<JArray>();
        
        var array = new JArray();
        variant = array;
        return array;
    }
}