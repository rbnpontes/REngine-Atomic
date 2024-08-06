using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BindingGenerator.Models;

namespace BindingGenerator.Generators
{
	internal class NoneGenerator(RunArguments _) : ICodeGenerator
	{
		public void Run()
		{
			Console.WriteLine("- No bind set. Please use --bind js|net instead.");
		}
	}
}
