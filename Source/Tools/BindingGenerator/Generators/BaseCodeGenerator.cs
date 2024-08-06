using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using BindingGenerator.Models;
using Mustache;
using Module = BindingGenerator.Models.Module;

namespace BindingGenerator.Generators
{
	internal abstract class BaseCodeGenerator(RunArguments arguments) : ICodeGenerator
	{
		protected readonly RunArguments mArguments = arguments;
		protected Module mModule = new();
		protected readonly List<ModuleItem> mModuleItems = new();

		public virtual void Run()
		{
			if (string.IsNullOrEmpty(arguments.InputDir))
				throw new Exception("Input Directory is required.");
			if (string.IsNullOrEmpty(arguments.OutputDir))
				throw new Exception("Output Directory is required.");
			if (string.IsNullOrEmpty(arguments.SourceDir))
				throw new Exception("Source Directory is required.");
			LoadModule();
			LoadModules();
			FixSourceFilePaths();
			
			Console.WriteLine($"- Found {mModule.Name} module with ({mModuleItems.Count}) items.");
		}

		protected virtual void LoadModule()
		{
			var moduleData = File.ReadAllText(arguments.InputDir);
			mModule = JsonSerializer.Deserialize<Module>(moduleData) ?? throw new NullReferenceException();
		}
		protected virtual void LoadModules()
		{
			mModuleItems.Clear();
			var modulePath = Path.GetDirectoryName(arguments.InputDir);
			foreach (var moduleItem in mModule.Modules)
			{
				var moduleItemPath = Path.Join(modulePath, moduleItem + ".json");
				var moduleItemData = File.ReadAllText(moduleItemPath);

				mModuleItems.Add(
					JsonSerializer.Deserialize<ModuleItem>(moduleItemData) ?? throw new NullReferenceException()
				);
			}	
		}

		protected virtual void FixSourceFilePaths()
		{
			var format = new FormatCompiler();
			foreach (var moduleItem in mModuleItems)
			{
				var model = new SourceMustacheModel(mModule, moduleItem);
				for (var i = 0; i < moduleItem.Sources.Length; ++i)
				{
					var src = moduleItem.Sources[i];
					var gen = format.Compile(src);
					
					src = gen.Render(model);
					
					var fixedSrc = Path.Join(mArguments.SourceDir, src);
					if(Directory.Exists(fixedSrc))
						moduleItem.Sources[i] = fixedSrc;
					else if (Directory.Exists(src))
						moduleItem.Sources[i] = src;
					else
						throw new DirectoryNotFoundException($"Not found source directory. Directory={src}");
				}
			}
		}
	}
}
