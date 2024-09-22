using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BindingGenerator.CodeBuilders
{
	public class FunctionDesc
	{
		public string Name { get; set; } = string.Empty;
		public string ReturnType { get; set; } = "void";
		public string[] Arguments { get; set; } = [];
	}
	public class CppBuilder(int indentation = 0)
	{
		private CodeBlockChunk pChunk = new(indentation);
		private List<string> pHeaders = new();
		private string pTopComment = string.Empty;
		private bool pPragmaOnce;
		
		public CppBuilder SetPragmaOnce()
		{
			pPragmaOnce = true;
			return this;
		}

		public CppBuilder SetTopComment(string comment)
		{
			pTopComment = comment;
			return this;
		}

		public CppBuilder Include(string includeName)
		{
			pHeaders.Add(includeName);
			return this;
		}
		public CppBuilder Namespace(Action<CppBuilder> namespaceBody, string ns)
		{
			pChunk.Add($"namespace {ns}");
			return Closure(namespaceBody);
		}

		public CppBuilder Comment(string comment)
		{
			return Line("//" + comment);
		}
		public CppBuilder Line(string line)
		{
			pChunk.Add(line);
			return this;
		}

		private string GetMethodDeclaration(FunctionDesc functionDesc)
		{
			StringBuilder result = new();
			result.Append(functionDesc.ReturnType);
			result.Append(' ');
			result.Append(functionDesc.Name);
			result.Append('(');
			for (var i = 0; i < functionDesc.Arguments.Length; ++i)
			{
				result.Append(functionDesc.Arguments[i]);
				if (i < functionDesc.Arguments.Length - 1)
					result.Append(", ");
			}

			result.Append(')');

			return result.ToString();
		}
		public CppBuilder MethodDecl(FunctionDesc functionDesc)
		{
			pChunk.Add($"{GetMethodDeclaration(functionDesc)};");
			return this;
		}
		public CppBuilder Method(Action<CppBuilder> methodBody, FunctionDesc functionDesc)
		{
			pChunk.Add($"{GetMethodDeclaration(functionDesc)}");
			Closure(methodBody);
			return this;
		}
		public CppBuilder MethodCall(string name, string[] args)
		{
			StringBuilder sb = new();
			sb.Append(name);
			sb.Append('(');
			for (var i = 0; i < args.Length; ++i)
			{
				sb.Append(args[i]);
				if (i < args.Length - 1)
					sb.Append(", ");
			}

			sb.Append(");");
			return Line(sb.ToString());
		}
		public CppBuilder Closure(Action<CppBuilder> closureBody)
		{
			var closure = new CppBuilder(indentation + 1);
			closure.Comment("Closure");
			closureBody(closure);
			
			pChunk.Add("{");
			pChunk.Add(closure.pChunk);
			pChunk.Add("}");
			return this;
		}

		public CppBuilder Var(string varType, string varName)
		{
			pChunk.Add($"{varType} {varName};");
			return this;
		}

		public CppBuilder Var(string varType, string varName, string varValue)
		{
			pChunk.Add($"{varType} {varName} = {varValue};");
			return this;
		}

		public CppBuilder Return()
		{
			pChunk.Add("return;");
			return this;
		}

		public CppBuilder Return(string resultValue)
		{
			pChunk.Add($"return {resultValue};");
			return this;
		}
		public override string ToString()
		{
			StringBuilder sb = new();
			if (!string.IsNullOrEmpty(pTopComment))
			{
				sb.AppendLine("/*");
				sb.AppendLine(pTopComment);
				sb.AppendLine("*/");
			}
			
            if (pPragmaOnce)
				sb.AppendLine("#pragma once");
			
			pHeaders.ForEach(x => sb.AppendLine($"#include {x}"));
			sb.AppendLine();

			sb.AppendLine(pChunk.GetString());
			return sb.ToString();
		}
	}
}
