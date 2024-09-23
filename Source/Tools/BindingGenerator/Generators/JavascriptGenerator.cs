using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Threading.Tasks;
using BindingGenerator.CodeBuilders;
using BindingGenerator.Models;
using BindingGenerator.Utils;

namespace BindingGenerator.Generators
{
	internal class JavascriptGenerator(RunArguments arguments) : BaseCodeGenerator(arguments)
	{
		public override void Run()
		{
			base.Run();
			Console.WriteLine("- Generating JavaScript Bindings");
			GenerateModuleBootstrapHeader();
			GenerateModuleBootstrapSource();

			if (mGlobalNamespace is null)
				return;
			
			GenerateEnumBindings(mGlobalNamespace);
			GenerateStructBindings(mGlobalNamespace);
			
			GenerateIndexHeader(Path.Join(mArguments.OutputDir, "Enums"));
			GenerateIndexHeader(Path.Join(mArguments.OutputDir, "Structs"));
		}

		private void GenerateModuleBootstrapHeader()
		{
			DuktapeBuilder rootBuilder = new();
			rootBuilder.SetPragmaOnce();

			rootBuilder.Namespace(builder =>
			{
				builder.MethodDecl(new FunctionDesc()
				{
					Arguments = ["duk_context* ctx"],
					ReturnType = "void",
					Name = CodeUtils.ToSnakeCase(mModule.Name)+"_module_setup"
				});
			}, "REngine");

			var outputPath = Path.Join(arguments.OutputDir, "Bootstrap.h");
			CodeUtils.WriteCode(outputPath, rootBuilder);
		}

		private void GenerateModuleBootstrapSource()
		{
			Action<CppBuilder> setupMethodBody = (methodBuilder) =>
			{
				methodBuilder.Comment("types setup");
				if (mGlobalNamespace is null)
					return;
				GenerateSetupTypesNamespaceCalls(methodBuilder, mGlobalNamespace);
			};
			
			DuktapeBuilder builder = new();
			builder
				.IncludeLiteral("Bootstrap.h")
				.IncludeLiteral("./Enums/Index.h")
				.IncludeLiteral("./Structs/Index.h")
				.IncludeLiteral("./Classes/Index.h")
				.IncludeLiteral("./StaticMethods/Index.h");
			
			builder.Namespace(builder =>
			{
				builder.Method(setupMethodBody, new FunctionDesc()
				{
					Arguments = ["duk_context* ctx"],
					ReturnType = "void",
					Name = CodeUtils.ToSnakeCase(mModule.Name) + "_module_setup"
				});
			}, "REngine");

			var outputPath = Path.Join(arguments.OutputDir, "Bootstrap.cpp");
			CodeUtils.WriteCode(outputPath, builder);
		}

		private void GenerateIndexHeader(string outputPath)
		{
			var indexPath = Path.Join(outputPath, "Index.h");
			
			if(File.Exists(indexPath))
				File.Delete(indexPath);

			var includes = Directory.GetFiles(outputPath)
				.Where(x => x.EndsWith(".h"))
				.Select(x => $"./{Path.GetFileName(x)}");
			
			var builder = new CppBuilder();
			builder.SetTopComment(BindingFileUtils.GeneratedFileComment);
			builder.SetPragmaOnce();
			foreach (var include in includes)
				builder.IncludeLiteral(include);
			
			CodeUtils.WriteCode(indexPath, builder);
		}
		
		private void GenerateSetupTypesNamespaceCalls(CppBuilder builder, NamespaceDefinition ns)
		{
			var nsDecl = CodeUtils.GetSnakeCaseNamespaceChain(ns);
			if (!string.IsNullOrEmpty(nsDecl))
				nsDecl += "_";
			
			foreach (var @enum in ns.Enums.NonRepeated(x => x.Name))
				builder.MethodCall(nsDecl + CodeUtils.ToSnakeCase(@enum.Name) + "_enum_setup", ["ctx"]);
			foreach (var @method in ns.Structs.NonRepeated(x => x.Name))
				builder.MethodCall(nsDecl + CodeUtils.ToSnakeCase(@method.Name) + "_struct_setup", ["ctx"]);
			foreach (var @class in ns.Classes.NonRepeated(x => x.Name))
				builder.MethodCall(nsDecl + CodeUtils.ToSnakeCase(@class.Name) + "_class_setup", ["ctx"]);
			foreach (var @method in ns.Methods.NonRepeated(x => x.Name))
				builder.MethodCall(nsDecl + CodeUtils.ToSnakeCase(@method.Name) + "_method_setup", ["ctx"]);

			foreach (var childNs in ns.Namespaces)
				GenerateSetupTypesNamespaceCalls(builder, childNs);
		}

		private void GenerateEnumBindings(NamespaceDefinition ns)
		{
			foreach (var @enum in ns.Enums)
			{
				GenerateEnumHeader(@enum);
				GenerateEnumSource(@enum);
			}

			foreach (var nsChild in ns.Namespaces)
				GenerateEnumBindings(nsChild);
		}

		private void GenerateEnumHeader(EnumDefinition @enum)
		{
			DuktapeBuilder builder = new();
			builder
				.SetPragmaOnce()
				.Include($"\"{@enum.HeaderFilePath}\"")
				.Namespace((builder) =>
				{
					builder.MethodDecl(new FunctionDesc()
					{
						Name = CodeUtils.GetMethodDeclName(@enum.Namespace, @enum)+"_enum_setup",
						ReturnType = "void",
						Arguments = ["duk_context* ctx"]
					});
				}, "REngine");

			
			var headerPath = Path.Join(mArguments.OutputDir, "Enums",
				BindingFileUtils.GetFileName(@enum.Namespace, @enum))+".h";
			CodeUtils.WriteCode(headerPath, builder);
		}

		private void GenerateEnumSource(EnumDefinition @enum)
		{
			var headerName = BindingFileUtils.GetFileName(@enum.Namespace, @enum)+".h";
			var setupMethodBody = (CppBuilder bodyBuilder)=>
			{
				// bodyBuilder hahahaha. Sorry!
				var builder = DuktapeBuilder.From(bodyBuilder);
				
				builder.PushObject();
				foreach (var entry in @enum.Entries)
					builder.PushInt(CodeUtils.GetEnumEntryAccessor(@enum, entry)).PutPropStringLiteral(-2, entry.Name);
				builder.PutGlobalStringLiteral(@enum.Name);
			};
			
			DuktapeBuilder builder = new();
			builder
				.SetPragmaOnce()
				.Include($"\"{headerName}\"")
				.Namespace((builder) =>
				{
					builder.Method(setupMethodBody, new FunctionDesc()
					{
						Name = CodeUtils.GetMethodDeclName(@enum.Namespace, @enum) + "_enum_setup",
						ReturnType = "void",
						Arguments = ["duk_context* ctx"]
					});
				}, "REngine");

			var sourcePath = Path.Join(mArguments.OutputDir, "Enums",
				BindingFileUtils.GetFileName(@enum.Namespace, @enum) + ".cpp");
			CodeUtils.WriteCode(sourcePath, builder);
		}
		
		private void GenerateStructBindings(NamespaceDefinition ns)
		{
			foreach (var @struct in ns.Structs)
			{
				GenerateStructHeader(@struct);
				GenerateStructSource(@struct);
			}

			foreach (var childNs in ns.Namespaces)
				GenerateStructBindings(childNs);
		}

		private void GenerateStructHeader(StructDefinition @struct)
		{
			var headerPath = Path.Join(mArguments.OutputDir, "Structs",
				BindingFileUtils.GetFileName(@struct.Namespace, @struct))+".h";
			var builder = new DuktapeBuilder();
			
			CodeUtils.WriteCode(headerPath, builder);
		}

		private void GenerateStructSource(StructDefinition @struct)
		{
			
		}
	}
}
