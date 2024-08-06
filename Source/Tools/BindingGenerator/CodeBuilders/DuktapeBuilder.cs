using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BindingGenerator.CodeBuilders
{
	internal class DuktapeBuilder
	{
		private List<string> pHeaders = new();
		private string pNamespace = string.Empty;
		private bool pIsHeader;
		private CodeBlockChunk pCurrChunk = new();

		public DuktapeBuilder SetAsHeader()
		{
			pIsHeader = true;
			return this;
		}

		public DuktapeBuilder Add(string line)
		{
			pCurrChunk.Add(line);
			return this;
		}

		public DuktapeBuilder Closure(Action call)
		{
			var lastChunk = pCurrChunk;
			pCurrChunk = new CodeBlockChunk();
			lastChunk.Add(pCurrChunk);

			call();

			pCurrChunk = lastChunk;
			return this;
		}

		public override string ToString()
		{
			StringBuilder sb = new StringBuilder();
			if (pIsHeader)
				sb.AppendLine("#pragma once");

			pHeaders.ForEach(x =>
			{
				sb.AppendLine($"#include {x}");
			});

			if (!string.IsNullOrEmpty(pNamespace))
			{
				var block = new CodeBlockChunk();
				block.Add($"namespace {pNamespace}");
				block.Add("{");
				block.Add(pCurrChunk);
				block.Add("}");

				sb.AppendLine(block.GetString());
			}
			else
			{
				sb.AppendLine(pCurrChunk.GetString());
			}

			return sb.ToString();
		}
	}
}
