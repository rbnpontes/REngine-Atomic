using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BindingGenerator.Models
{
	internal class RunArguments
	{
		public string SourceDir { get; set; } = string.Empty;
		public string OutputDir { get; set; } = string.Empty;
		public string InputDir { get; set; } = string.Empty;
		public BindingType BindingType { get; set; } = BindingType.None;
	}
}
