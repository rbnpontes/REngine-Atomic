using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Threading.Tasks;
using BindingGenerator.CodeBuilders;
using BindingGenerator.Models;

namespace BindingGenerator.Generators
{
	internal class JavascriptGenerator(RunArguments arguments) : BaseCodeGenerator(arguments)
	{
		public override void Run()
		{
			base.Run();
			Console.WriteLine("- Generating JavaScript Bindings");
		}
	}
}
