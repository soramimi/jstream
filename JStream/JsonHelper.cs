using System;
using System.Globalization;
using System.Text;

namespace JStream;

internal static class JsonHelper
{
    public static string EncodeJsonString(string input)
    {
        if (string.IsNullOrEmpty(input))
            return string.Empty;

        var sb = new StringBuilder(input.Length + 10);
        
        foreach (char c in input)
        {
            switch (c)
            {
                case '"':
                    sb.Append("\\\"");
                    break;
                case '\\':
                    sb.Append("\\\\");
                    break;
                case '\b':
                    sb.Append("\\b");
                    break;
                case '\f':
                    sb.Append("\\f");
                    break;
                case '\n':
                    sb.Append("\\n");
                    break;
                case '\r':
                    sb.Append("\\r");
                    break;
                case '\t':
                    sb.Append("\\t");
                    break;
                default:
                    if (c >= 0x20 && c < 0x7f)
                    {
                        sb.Append(c);
                    }
                    else
                    {
                        sb.Append($"\\u{(int)c:X4}");
                    }
                    break;
            }
        }
        
        return sb.ToString();
    }

    public static double ParseNumber(string numberString, bool allowHexadecimal = false, bool allowSpecialConstants = false)
    {
        if (string.IsNullOrEmpty(numberString))
            return 0.0;

        numberString = numberString.Trim();

        if (allowHexadecimal && numberString.StartsWith("0x", StringComparison.OrdinalIgnoreCase))
        {
            if (long.TryParse(numberString[2..], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out long hexValue))
                return hexValue;
        }

        if (allowSpecialConstants)
        {
            bool isNegative = numberString.StartsWith('-');
            string valueString = isNegative ? numberString[1..] : numberString;
            
            if (string.Equals(valueString, "Infinity", StringComparison.OrdinalIgnoreCase))
                return isNegative ? double.NegativeInfinity : double.PositiveInfinity;
            
            if (string.Equals(valueString, "NaN", StringComparison.OrdinalIgnoreCase))
                return double.NaN;
        }

        if (double.TryParse(numberString, NumberStyles.Float, CultureInfo.InvariantCulture, out double result))
            return result;

        return 0.0;
    }

    public static string FormatNumber(double value, bool allowNaN = false)
    {
        if (double.IsNaN(value))
            return allowNaN ? "NaN" : "null";
        
        if (double.IsPositiveInfinity(value))
            return allowNaN ? "Infinity" : "null";
        
        if (double.IsNegativeInfinity(value))
            return allowNaN ? "-Infinity" : "null";

        return value.ToString("G17", CultureInfo.InvariantCulture);
    }
}