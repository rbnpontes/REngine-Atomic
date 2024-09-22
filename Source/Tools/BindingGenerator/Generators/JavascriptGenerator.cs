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
				GenerateSetupTypesNamespaceCalls(methodBuilder, mGlobalNamespace, string.Empty);
			};
			
			DuktapeBuilder builder = new();
			builder.Include("\"Bootstrap.h\"");
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

		private void GenerateSetupTypesNamespaceCalls(CppBuilder builder, NamespaceDefinition ns, string prevNamespaceDecl)
		{
			var nsDecl = prevNamespaceDecl + CodeUtils.ToSnakeCase(ns.Name)+"_";
			if (ns == mGlobalNamespace)
				nsDecl = string.Empty;
			
			foreach (var @enum in ns.Enums)
				builder.MethodCall(nsDecl + CodeUtils.ToSnakeCase(@enum.Name) + "_enum_setup", ["ctx"]);
			foreach (var @method in ns.Structs)
				builder.MethodCall(nsDecl + CodeUtils.ToSnakeCase(@method.Name) + "_struct_setup", ["ctx"]);
			foreach (var @class in ns.Classes)
				builder.MethodCall(nsDecl + CodeUtils.ToSnakeCase(@class.Name) + "_class_setup", ["ctx"]);
			foreach (var @method in ns.Methods)
				builder.MethodCall(nsDecl + CodeUtils.ToSnakeCase(@method.Name) + "_method_setup", ["ctx"]);

			foreach (var childNs in ns.Namespaces)
				GenerateSetupTypesNamespaceCalls(builder, childNs, nsDecl);
		}
	}
}
