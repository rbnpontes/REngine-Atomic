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

        protected NamespaceDefinition? mGlobalNamespace;
        protected readonly Dictionary<string, (TypeDefinition, CppElement)> mTypes = new();

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
            
            var astTargetPath = Path.Join(arguments.OutputDir, "ast.json");
            if (File.Exists(astTargetPath))
            {
                Console.WriteLine("- Found Serialized AST Data. Loading and Skipping AST Build");
                using var astStream = new FileStream(astTargetPath, FileMode.Open);
                var deserializer = new TypeDefinitionDeserializer();
                deserializer.Deserialize(astStream);

                mGlobalNamespace = deserializer.Namespace;
                Console.WriteLine("- Deserialize was success");
                Console.WriteLine("- Total of Types: " + deserializer.TotalOfTypes);
                return;
            }
            
            Console.WriteLine($"- Building AST");
            var compilation = BuildSourcesAst();
            Console.WriteLine("- Collect Type Definitions");
            var typeCollector = new TypeCollector(
                new TypeCollectorCreateDesc {
                    Compilation = compilation,
                    Module = mModule,
                    ModuleItems = mSourceFiles
                }
            );
            typeCollector.Collect();
            mGlobalNamespace = typeCollector.Namespace;
            
            Console.WriteLine("- Total of Types: " + typeCollector.TotalOfTypes);

            if (!arguments.SerializeAst) 
                return;
            
            Console.WriteLine("- Serializing Types AST.");
            if(File.Exists(astTargetPath))
                File.Delete(astTargetPath);
                
            using var stream = new FileStream(astTargetPath, FileMode.CreateNew);
            
            var serializer = new TypeDefinitionSerializer(typeCollector.Namespace);
            serializer.Build();
            serializer.Serialize(stream);
        }

        protected virtual void LoadModule()
        {
            var moduleData = File.ReadAllText(arguments.InputDir);
            mModule = JsonSerializer.Deserialize<Module>(moduleData) ?? throw new NullReferenceException();
            if (string.IsNullOrEmpty(mModule.Source))
                throw new Exception("Module source path is required.");
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
                    if (Directory.Exists(fixedSrc))
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

            var items = mSourceFiles.Select(x => x.Item2).Where(x => !x.EndsWith("Index.h"));
            var compilation = CppParser.ParseFiles(items.ToList(), parserOptions);

            var messages = compilation.Diagnostics.Messages.OrderBy(x => x.Type);
            foreach (var msg in messages)
            {
                // Skip warning logs
                if (!mArguments.EnableWarnings && msg.Type == CppLogMessageType.Warning)
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
    }
}