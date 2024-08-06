using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BindingGenerator.CodeBuilders
{
	interface ICodeChunk
	{
		int Indentation { get; set; }
		string GetString();
	}

	abstract class BaseCodeChunk : ICodeChunk
	{
		private int pIndentation = 0;

		public int Indentation
		{
			get => pIndentation;
			set
			{
				var currValue = pIndentation;
				pIndentation = value;
				if(currValue != value)
					OnIndentationChange(currValue);
			}
		}
		public abstract string GetString();
		public virtual string GetIndentation()
		{
			var arr = new char[Indentation];
			Array.Fill(arr, '\t');
			return new string(arr);
		}

		protected virtual void OnIndentationChange(int prevValue)
		{
		}
	}

	class CodeChunk : BaseCodeChunk
	{
		public string Code { get; set; }
		public CodeChunk(string code, int indentation = 0)
		{
			Code = code;
			Indentation = indentation;
		}

		public CodeChunk(string code, ICodeChunk chunk)
		{
			Code = code;
			Indentation = chunk.Indentation + 1;
		}

		public void Append(string code)
		{
			Code += code;
		}
		public override string GetString()
		{
			return GetIndentation() + Code;
		}
	}

	class CodeBlockChunk : BaseCodeChunk
	{
		private readonly List<ICodeChunk> pChunks = new();

		public CodeBlockChunk(int indentation = 0)
		{
			Indentation = indentation;
		}

		public void Add(ICodeChunk chunk)
		{
			chunk.Indentation = Indentation + 1;
			pChunks.Add(chunk);
		}

		public void Add(string code)
		{
			Add(new CodeChunk(code));
		}

		public void Add(IEnumerable<ICodeChunk> chunks)
		{
			foreach (var chunk in chunks)
				Add(chunk);
		}

		protected override void OnIndentationChange(int prevValue)
		{
			pChunks.ForEach(x => x.Indentation = Indentation + 1);
		}

		public override string GetString()
		{
			StringBuilder sb = new();
			pChunks.ForEach(x =>
			{
				sb.Append(GetIndentation());
				sb.Append(x.GetString());
				sb.AppendLine();
			});

			return sb.ToString();
		}
	}
}
