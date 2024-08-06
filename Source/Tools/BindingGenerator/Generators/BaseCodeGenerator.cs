using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using System.Xml.Serialization;
using BindingGenerator.Models;
using CppAst;
using Mustache;
using Module = BindingGenerator.Models.Module;

namespace BindingGenerator.Generators
{
	internal abstract class BaseCodeGenerator(RunArguments arguments) : ICodeGenerator
	{
		private readonly object pLoggerSync = new();
		protected readonly RunArguments mArguments = arguments;
		protected Module mModule = new();
		protected readonly List<ModuleItem> mModuleItems = new();
		protected readonly List<(ModuleItem, string)> mSourceFiles = new();

		protected GlobalNamespaceDefinition? mGlobalNamespace;
		protected readonly Dictionary<string, TypeDefinition> mTypes = new();

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

			LoadSourceFiles();
			Console.WriteLine($"- Found {mSourceFiles.Count} header files.");
			Console.WriteLine($"- Building AST");
			var compilation = BuildSourcesAst();
			Console.WriteLine("- Collect Type Definitions");
			CollectTypeDefinitions(compilation);
			Console.WriteLine("- Total of Types: "+mTypes.Count);
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

			var includeSet = new HashSet<string>();
			for (var i = 0; i < mModule.Includes.Length; ++i)
			{
				var include = mModule.Includes[i];
				var gen = format.Compile(include);
				// Parse mustache patterns
				include = gen.Render(new
				{
					Module = mModule,
					source = mArguments.SourceDir
				});

				include = Path.GetFullPath(include);
				includeSet.Add(include);
			}
			
			// Get only unique includes. Repeated includes will be discarded
			mModule.Includes = includeSet.ToArray();
			
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
						moduleItem.Sources[i] = Path.GetFullPath(fixedSrc);
					else if (Directory.Exists(src))
						moduleItem.Sources[i] = src;
					else
						throw new DirectoryNotFoundException($"Not found source directory. Directory={src}");
				}
			}
		}

		protected virtual void LoadSourceFiles()
		{
			mSourceFiles.Clear();

			foreach (var moduleItem in mModuleItems)
			foreach (var sourcePath in moduleItem.Sources)
			{
				var files = Directory.GetFiles(sourcePath)
					.Where(x => x.EndsWith(".h"))
					.Select(x => (moduleItem, x));
				mSourceFiles.AddRange(files);
			}
		}
		protected virtual CppCompilation BuildSourcesAst()
		{
			var parserOptions = new CppParserOptions();
			parserOptions.AdditionalArguments.AddRange([
				"-std=c++17",
				"-Wno-ignored-qualifiers",
			]);
			parserOptions.IncludeFolders.AddRange(mModule.Includes);
			parserOptions.Defines.AddRange(mModule.Defines);
			parserOptions.Defines.Add("ENGINE_BINDING_TOOL=1");
			
			var items = mSourceFiles.Select(x => x.Item2);
			var compilation = CppParser.ParseFiles(items.ToList(), parserOptions);

			var messages = compilation.Diagnostics.Messages.OrderBy(x => x.Type);
			foreach (var msg in messages)
			{
				// Skip warning logs
				if(!mArguments.EnableWarnings && msg.Type == CppLogMessageType.Warning)
					continue;
				
				var msgType = "LOG";
				var color = Console.ForegroundColor;
				if (msg.Type == CppLogMessageType.Error)
				{
					msgType = "ERROR";
					color = ConsoleColor.Red;
				}
				else if (msg.Type == CppLogMessageType.Warning)
				{
					msgType = "WARNING";
					color = ConsoleColor.Yellow;
				}

				var defaultColor = Console.ForegroundColor;
				Console.ForegroundColor = color;
				Console.Write($"[{msgType}]: ");
				Console.ForegroundColor = defaultColor;
				Console.WriteLine(msg.Text);
				Console.WriteLine($"Source: {msg.Location.File}({msg.Location.Line}, {msg.Location.Column})");
			}
			
			if (compilation.HasErrors)
				throw new Exception("Failed to Build AST. See Logs!");
			
			Console.WriteLine("- Finished AST Build");

			return compilation;
		}

		protected virtual void CollectTypeDefinitions(CppCompilation compilation)
		{
			mTypes.Clear();

			Dictionary<string, ModuleItem> allowedSrcFiles = new(mSourceFiles.Select(x => new KeyValuePair<string, ModuleItem>(x.Item2, x.Item1)));
			var currentNamespace = new GlobalNamespaceDefinition(mModule, null);
			mGlobalNamespace = currentNamespace;
			
			CollectEnums(compilation, allowedSrcFiles, currentNamespace);
			CollectClasses(compilation, allowedSrcFiles, currentNamespace);
		}

		protected virtual void CollectEnums(CppCompilation compilation, Dictionary<string, ModuleItem> allowedSourceFiles, NamespaceDefinition @namespace)
		{
			var enums = new List<EnumDefinition>();
			foreach (var @enum in compilation.Enums)
			{
				// Only process mapped source files. Other modules will be skipped
				var srcFile = Path.GetFullPath(@enum.SourceFile);
				if(!allowedSourceFiles.TryGetValue(srcFile, out var moduleItem))
					continue;

				var enumDef = new EnumDefinition(mModule, moduleItem);
				enumDef.Name = @enum.GetDisplayName();
				enumDef.Comment = @enum.Comment.ToString();
				enumDef.HeaderFilePath = srcFile;
				enumDef.Entries = @enum.Items
					.Select(enumItem => new EnumEntry(enumDef)
					{
						Comment = enumItem.Comment.ToString(), 
						Name = enumItem.Name, 
						Value = (int)enumItem.Value
					}).ToArray();
				enums.Add(enumDef);
			}

			@namespace.Enums = enums.ToArray();
		}

		protected virtual void CollectClasses(CppCompilation compilation,
			Dictionary<string, ModuleItem> allowedSourceFiles, NamespaceDefinition @namespace)
		{
			var classes = new List<ClassDefinition>();
			foreach (var @class in compilation.Classes)
			{
				// Only process mapped source files. Other modules will be skipped
				var srcFile = Path.GetFullPath(@class.SourceFile);
				if(!allowedSourceFiles.TryGetValue(srcFile, out var moduleItem))
					continue;

				var classDef = new ClassDefinition(mModule, moduleItem, @namespace);
				classDef.Name = @class.GetDisplayName();
				classDef.Comment = @class.Comment.ToString();
				classDef.HeaderFilePath = srcFile;
				
				classes.Add(classDef);
			}

			@namespace.Classes = classes.ToArray();
		}
	}
}
