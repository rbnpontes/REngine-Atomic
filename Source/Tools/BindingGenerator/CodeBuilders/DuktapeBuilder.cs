using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BindingGenerator.CodeBuilders
{
	public class DuktapeBuilder : CppBuilder
	{
		public DuktapeBuilder() : base()
		{
			SetTopComment("Don't touch on this file, the file is generated automatically\nby the binding generator.");
			Include("<Duktape/duktape.h>");
		}
	}
}
