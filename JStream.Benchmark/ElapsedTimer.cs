using System.Diagnostics;

namespace JStream.Benchmark;

/// <summary>
/// Simple elapsed time measurement utility, equivalent to C++ ElapsedTimer
/// </summary>
public class ElapsedTimer {
	private Stopwatch _stopwatch = new();

	/// <summary>
	/// Start measuring elapsed time
	/// </summary>
	public void Start()
	{
		_stopwatch.Restart();
	}

	/// <summary>
	/// Get elapsed time in milliseconds since Start() was called
	/// </summary>
	/// <returns>Elapsed time in milliseconds</returns>
	public long Elapsed()
	{
		return _stopwatch.ElapsedMilliseconds;
	}
}
