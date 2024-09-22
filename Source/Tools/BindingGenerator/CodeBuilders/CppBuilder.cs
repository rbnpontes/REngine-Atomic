using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BindingGenerator.CodeBuilders
{
	class FunctionDesc
	{
		public string MethodName { get; set; } = string.Empty;
		public string ReturnType { get; set; } = "void";
		public string[] Arguments { get; set; } = [];
	}
	internal class CppBuilder(int indentation = 0)
	{
		private CodeBlockChunk pChunk = new(indentation);
		private List<string> pHeaders = new();
		private bool pPragmaOnce;
		
		public CppBuilder SetPragmaOnce()
		{
			pPragmaOnce = true;
			return this;
		}
		public CppBuilder Namespace(Action<CppBuilder> callback, string ns)
		{
			pChunk.Add($"namespace {ns}");	
			pChunk.Add("{");
			var builder = new CppBuilder(indentation + 1);
			pChunk.Add(builder.ToString());
			pChunk.Add("}");
			return this;
		}

		public CppBuilder AddMethod(string methodName)
		{
			return this;
		}
		public CppBuilder AddMethod(string returnType, string methodName)
		{
			return this;
		}

		public override string ToString()
		{
			StringBuilder sb = new();
			if (pPragmaOnce)
				sb.AppendLine("#pragma once");
			
			pHeaders.ForEach(x => sb.AppendLine($"#include {x}"));
			sb.AppendLine();

			sb.AppendLine(pChunk.ToString());
			return sb.ToString();
		}
	}
}
