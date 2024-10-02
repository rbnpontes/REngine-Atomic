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

		private string GetModuleBindingIndex()
		{
			return $"<{mModule.Name}/Index.h>";
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
				.IncludeLiteral("Export.h")
				.IncludeLiteral("Bootstrap.h")
				.IncludeLiteral("./Enums/Index.h")
				.IncludeLiteral("./Structs/Index.h");
				// .IncludeLiteral("./Classes/Index.h")
				// .IncludeLiteral("./StaticMethods/Index.h");
			
			builder.Namespace(builder =>
			{
				var setupFnName = CodeUtils.ToSnakeCase(mModule.Name) + "_module_setup";
				var unloadFnName = CodeUtils.ToSnakeCase(mModule.Name) + "_module_unload";
				builder.Method(setupMethodBody, new FunctionDesc()
				{
					Arguments = ["duk_context* ctx"],
					ReturnType = "void",
					Name = setupFnName
				});
				builder.Method((_) => { }, new FunctionDesc()
				{
					Arguments = ["duk_context* ctx"],
					ReturnType = "void",
					Name = unloadFnName
				});
				builder.Line($"export_module({mModule.Name}, {setupFnName}, {unloadFnName});");
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
			foreach (var @struct in ns.Structs.NonRepeated(x => x.Name))
				builder.MethodCall(nsDecl + CodeUtils.ToSnakeCase(@struct.Name) + "_struct_setup", ["ctx"]);
			// foreach (var @class in ns.Classes.NonRepeated(x => x.Name))
			// 	builder.MethodCall(nsDecl + CodeUtils.ToSnakeCase(@class.Name) + "_class_setup", ["ctx"]);
			// foreach (var @method in ns.Methods.NonRepeated(x => x.Name))
			// 	builder.MethodCall(nsDecl + CodeUtils.ToSnakeCase(@method.Name) + "_method_setup", ["ctx"]);

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
				builder.AssertHeap().UsingNamespace(@enum.Namespace).Line();
				
				builder.PushObject();
				foreach (var entry in @enum.Entries)
					builder.PushInt($"static_cast<duk_int_t>({CodeUtils.GetEnumEntryAccessor(@enum, entry)})").PutPropStringLiteral(-2, entry.Name);
				builder.PutPropStringLiteral(-2, @enum.Name);
			};
			
			DuktapeBuilder builder = new();
			builder
				.IncludeLiteral(headerName)
				.IncludeLiteral("Utils.h")
				.Include(GetModuleBindingIndex())
				.Namespace((builder) =>
				{
					var methodDecl = CodeUtils.GetMethodDeclName(@enum.Namespace, @enum);
					builder.Method(setupMethodBody, new FunctionDesc()
					{
						Name = methodDecl + "_enum_setup",
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
			var baseMethodSignature = CodeUtils.GetMethodDeclName(@struct.Namespace, @struct);
			var builder = new DuktapeBuilder();
			builder
				.SetPragmaOnce()
				.Namespace((builder) =>
				{
					builder.MethodDecl(new FunctionDesc()
					{
						Name = baseMethodSignature + "_struct_from",
						ReturnType = "duk_idx_t",
						Arguments = ["duk_context* ctx", "void* ptr"]
					});
					builder.MethodDecl(new FunctionDesc()
					{
						Name = baseMethodSignature + "_struct_setup",
						ReturnType = "void",
						Arguments = ["duk_context* ctx"]
					});
				}, "REngine");
			CodeUtils.WriteCode(headerPath, builder);
		}

		private void GenerateStructSource(StructDefinition @struct)
		{
			var headerName = BindingFileUtils.GetFileName(@struct.Namespace, @struct) + ".h";
			var sourcePath = Path.Join(mArguments.OutputDir, "Structs",
				BindingFileUtils.GetFileName(@struct.Namespace, @struct))+".cpp";

			var baseMethodSignature = CodeUtils.GetMethodDeclName(@struct.Namespace, @struct);
			var ctorBody = (CppBuilder builder) =>
			{
				GenerateStructCtor(@struct, DuktapeBuilder.From(builder));
			};
			var fromBody = (CppBuilder builder) =>
			{
				GenerateStructFromCall(@struct, DuktapeBuilder.From(builder));
			};
			var finalizerBody = (CppBuilder builder) =>
			{
				GenerateStructFinalizerCall(@struct, DuktapeBuilder.From(builder));
			};
			var functionBody = (CppBuilder funcBuilder) =>
			{
				var builder = DuktapeBuilder.From(funcBuilder);
				builder.AssertHeap().UsingNamespace(@struct.Namespace).Line();
				
				builder
					.PushFunction($"{baseMethodSignature}_struct_ctor", 0)
					.PutPropStringLiteral(-2, @struct.Name);
			};
			
			var builder = new DuktapeBuilder();
			builder
				.IncludeLiteral(headerName)
				.IncludeLiteral("Utils.h")
				.IncludeLiteral("Bookkeeper.h")
				.Include(GetModuleBindingIndex())
				.Namespace((builder) =>
				{
					builder.MethodDecl(new FunctionDesc()
					{
						Name = baseMethodSignature + "_struct_define",
						ReturnType = "void",
						Arguments = ["duk_context* ctx", $"{CodeUtils.GetStructAccessor(@struct)}* instance"]
					});
					builder.Method(fromBody, new FunctionDesc()
					{
						Name = baseMethodSignature + "_struct_from",
						ReturnType = "duk_idx_t",
						Arguments = ["duk_context* ctx", "void* ptr"]
					});

					builder.Method(finalizerBody, new FunctionDesc()
					{
						Name = baseMethodSignature + "_struct_finalizer",
						ReturnType = "duk_idx_t",
						Arguments = ["duk_context* ctx"]
					});
					
					builder.Method(ctorBody, new FunctionDesc()
					{
						Name = baseMethodSignature+"_struct_ctor",
						ReturnType = "duk_idx_t",
						Arguments = ["duk_context* ctx"]
					});
					builder.Method(functionBody, new FunctionDesc()
					{
						Name = baseMethodSignature+"_struct_setup",
						ReturnType = "void",
						Arguments = ["duk_context* ctx"]
					});
				}, "REngine");
			CodeUtils.WriteCode(sourcePath, builder);
		}

		private void GenerateStructCtor(StructDefinition @struct, DuktapeBuilder builder)
		{
			var baseMethodSignature = CodeUtils.GetMethodDeclName(@struct.Namespace, @struct);

			builder.Line($"auto instance = new {CodeUtils.GetStructAccessor(@struct)}();");
			builder.PushThis().NativeStorePointer(-1, "instance");
			builder.Line("bookkeeper_store(ctx, -1, instance);");
			// set sinalizer
			builder.Comment("setup finalizer");
			builder
				.PushFunction(baseMethodSignature + "_struct_finalizer", 0)
				.NativeStorePointer(-1, "instance")
				.SetFinalizer(-2);
			builder.Return("1");
		}

		private void GenerateStructFromCall(StructDefinition @struct, DuktapeBuilder builder)
		{
			builder.AssertHeap(1);
			builder.Line("if (bookkeeper_restore(ctx, ptr))");
			builder.Line("\treturn 1;");
			builder.Line();
			builder.TypeGet(@struct);
			builder.New(0);
			builder.Return("1");
		}

		private void GenerateStructFinalizerCall(StructDefinition @struct, DuktapeBuilder builder)
		{
			builder
				.PushCurrentFunction()
				.NativeGetPointer("void* ptr", -1)
				.Pop();
			builder.Line("if (ptr)");
			builder.Closure(x =>
			{
				x.Line("bookkeeper_remove(ctx, ptr);");
				x.Line($"delete static_cast<{CodeUtils.GetStructAccessor(@struct)}*>(ptr);");
			});
			builder.Return("0");
		}
	}
}
