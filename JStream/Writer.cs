using System;
using System.Collections.Generic;
using System.Text;

namespace JStream;

public class Writer
{
    private readonly Action<string> _output;
    private readonly List<int> _stack = new();
    private bool _enableIndent = true;
    private bool _enableNewline = true;
    private bool _allowNaN = false;

    public Writer(Action<string>? output = null)
    {
        _output = output ?? Console.Write;
        _stack.Add(0);
    }

    public bool EnableIndent
    {
        get => _enableIndent;
        set => _enableIndent = value;
    }

    public bool EnableNewline
    {
        get => _enableNewline;
        set => _enableNewline = value;
    }

    public bool AllowNaN
    {
        get => _allowNaN;
        set => _allowNaN = value;
    }

    private void Print(string text) => _output(text);
    private void Print(char ch) => _output(ch.ToString());

    private void PrintNewline()
    {
        if (!_enableNewline) return;
        Print('\n');
    }

    private void PrintIndent()
    {
        if (!_enableIndent) return;
        
        int depth = _stack.Count - 1;
        for (int i = 0; i < depth; i++)
        {
            Print("  ");
        }
    }

    private bool PrintNumber(double value)
    {
        string formatted = JsonHelper.FormatNumber(value, _allowNaN);
        if (formatted == "null" && !_allowNaN)
        {
            Print("null");
            return false;
        }
        
        Print(formatted);
        return true;
    }

    private void PrintString(string value)
    {
        Print('"');
        Print(JsonHelper.EncodeJsonString(value));
        Print('"');
    }

    private bool PrintValue(string name, Func<bool> fn)
    {
        PrintName(name);
        bool result = fn();
        
        if (_stack.Count > 0)
            _stack[^1]++;
        
        return result;
    }

    private void PrintObject(string name = "", Action? fn = null)
    {
        PrintName(name);
        Print('{');
        _stack.Add(0);
        
        if (fn != null)
        {
            fn();
            EndObject();
        }
    }

    private void PrintArray(string name = "", Action? fn = null)
    {
        PrintName(name);
        Print('[');
        _stack.Add(0);
        
        if (fn != null)
        {
            fn();
            EndArray();
        }
    }

    private void EndBlock()
    {
        PrintNewline();
        
        if (_stack.Count > 0)
        {
            _stack.RemoveAt(_stack.Count - 1);
            if (_stack.Count > 0)
                _stack[^1]++;
        }
        
        PrintIndent();
    }

    public void PrintName(string name)
    {
        if (_stack.Count > 0 && _stack[^1] > 0)
            Print(',');
        
        if (_stack.Count > 1)
            PrintNewline();
        
        PrintIndent();
        
        if (!string.IsNullOrEmpty(name))
        {
            PrintString(name);
            Print(':');
            if (_enableIndent)
                Print(' ');
        }
    }

    public void StartObject(string name = "")
    {
        PrintObject(name);
    }

    public void EndObject()
    {
        EndBlock();
        Print('}');
    }

    public void Object(string name, Action fn)
    {
        PrintObject(name, fn);
    }

    public void StartArray(string name = "")
    {
        PrintArray(name);
    }

    public void EndArray()
    {
        EndBlock();
        Print(']');
    }

    public void Array(string name, Action fn)
    {
        PrintArray(name, fn);
    }

    public bool Number(string name, double value)
    {
        return PrintValue(name, () => PrintNumber(value));
    }

    public void Number(double value)
    {
        Number("", value);
    }

    public void String(string name, string value)
    {
        PrintValue(name, () =>
        {
            PrintString(value);
            return true;
        });
    }

    public void String(string value)
    {
        String("", value);
    }

    public void Symbol(string name, StateType value)
    {
        PrintValue(name, () =>
        {
            string text = value switch
            {
                StateType.False => "false",
                StateType.True => "true",
                _ => "null"
            };
            Print(text);
            return true;
        });
    }

    public void Boolean(string name, bool value)
    {
        Symbol(name, value ? StateType.True : StateType.False);
    }

    public void Boolean(bool value)
    {
        Boolean("", value);
    }

    public void Null(string name = "")
    {
        Symbol(name, StateType.Null);
    }

    public void WriteVariant(string name, Variant variant)
    {
        if (variant.IsNull)
        {
            Null(name);
        }
        else if (variant.IsBoolean)
        {
            Boolean(name, variant.Get<bool>());
        }
        else if (variant.IsNumber)
        {
            Number(name, variant.Get<double>());
        }
        else if (variant.IsString)
        {
            String(name, variant.Get<string>());
        }
        else if (variant.IsObject)
        {
            var obj = variant.Get<JObject>();
            Object(name, () =>
            {
                foreach (var kvp in obj)
                {
                    WriteVariant(kvp.Key, kvp.Value);
                }
            });
        }
        else if (variant.IsArray)
        {
            var array = variant.Get<JArray>();
            Array(name, () =>
            {
                foreach (var item in array)
                {
                    WriteVariant("", item);
                }
            });
        }
    }

    public void WriteVariant(Variant variant)
    {
        WriteVariant("", variant);
    }

    public void Finish()
    {
        if (_stack.Count > 0 && _stack[0] > 0)
            PrintNewline();
    }
}