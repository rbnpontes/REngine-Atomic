// See https://aka.ms/new-console-template for more information

using BindingGenerator.Generators;
using BindingGenerator.Models;

namespace BindingGenerator;

class Program
{
	public static void Main(string[] args)
	{
		RunArguments arguments = ArgumentsProcessor.Process(args);
		var generators = new ICodeGenerator[]
		{
			new NoneGenerator(arguments),
			new JavascriptGenerator(arguments),
			new DotNetGenerator(arguments)
		};

		var gen = generators[(int)arguments.BindingType];
		gen.Run();
	}
}