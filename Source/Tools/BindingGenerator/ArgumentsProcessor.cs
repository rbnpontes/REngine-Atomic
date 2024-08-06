using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BindingGenerator.Models;

namespace BindingGenerator
{
	internal class ArgumentsProcessor
	{
		private static readonly int sJavaScriptBindKeyHash = string.GetHashCode("js");
		private static readonly int sNetBindKeyHash = string.GetHashCode("net");
		public static RunArguments Process(string[] args)
		{
			RunArguments result = new();
			var executeCalls = new Dictionary<string, Func<int, int>>()
			{
				{
					"--src", (argIdx) =>
					{
						var srcIdx = argIdx + 1;
						if (srcIdx >= args.Length)
							throw new Exception("Invalid Argument Source. Expected: --src PATH");
						result.SourceDir = args[srcIdx];

						var validPaths = new[]
						{
							result.SourceDir,
							Path.Combine(result.SourceDir, "Source"),
							Path.Combine(result.SourceDir, "Source", "EngineCore"),
							Path.Combine(result.SourceDir, "Artifacts"),
							Path.Combine(result.SourceDir, "Script")
						};

						if (validPaths.Any(validPath => !Directory.Exists(validPath)))
						{
							throw new Exception(
								"Invalid Argument Source. --src arg must be a valid path to source directory");
						}

						return srcIdx;
					}
				},
				{
					"--output", (argIdx) =>
					{
						var outIdx = argIdx + 1;
						if (outIdx >= args.Length)
							throw new Exception("Invalid Argument Output. Expected: --output PATH");
						result.OutputDir = args[outIdx];

						if (!Directory.Exists(result.OutputDir))
							throw new Exception(
								"Invalid Argument Output. --output must be a valid path to output directory.");

						return outIdx;
					}
				},
				{
					"--input", (argIdx) =>
					{
						var inputIdx = argIdx + 1;
						if (inputIdx >= args.Length)
							throw new Exception("Invalid Argument Input. Expected --input PATH/module.json");

						result.InputDir = args[inputIdx];

						if (!File.Exists(result.InputDir))
							throw new Exception("Invalid Argument Input. --input be a valid module.json");

						return inputIdx;
					}
				},
				{
					"--bind", (argIdx) =>
					{
						var bindIdx = argIdx + 1;
						if (argIdx >= args.Length)
							throw new Exception("Invalid Argument Bind. Expected --bind js or --bind net");

						var bind = string.GetHashCode(args[bindIdx]);
						if (bind == sJavaScriptBindKeyHash)
							result.BindingType = BindingType.Javascript;
						else if (bind == sNetBindKeyHash)
							result.BindingType = BindingType.DotNet;
						
						return bindIdx;
					}
				},
				{
					"--help", (idx) =>
					{
						Console.WriteLine("+============================+");
						Console.WriteLine("!                            !");
						Console.WriteLine("!       HELP COMMANDS        !");
						Console.WriteLine("!                            !");
						Console.WriteLine("+============================+");
						Console.WriteLine("");
						Console.WriteLine("--bind js|net : Bind generator type. (Javascript=js | DotNet(C#)=net)");
						Console.WriteLine("--src PATH/ : Path to engine source code directory");
						Console.WriteLine("--input PATH/module.json : Path to module.json that contains binding info.");
						Console.WriteLine("--output: PATH/ : Path where tool will output generated binding files.");

						// Reset run arguments
						result = new RunArguments();
						return args.Length;
					}
				}
			};

			for (var i = 0; i < args.Length; ++i)
			{
				if (!executeCalls.TryGetValue(args[i], out var call))
					continue;
				i = call(i);
			}
			return result;
		}
	}
}
