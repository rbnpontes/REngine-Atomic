using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

using ExcludedObjectMethods = System.Collections.Generic.Dictionary<string, string[]>;
namespace BindingGenerator.Models
{
	public class ExcludedObjectCollection : Dictionary<string, ExcludedObjectMethods>{}

	public class ModuleItem
	{
		[JsonPropertyName("name")]
		public string Name { get; set; } = string.Empty;
		[JsonPropertyName("sources")]
		public string[] Sources { get; set; } = [];
		[JsonPropertyName("includes")]
		public string[] Includes { get; set; } = [];
		[JsonPropertyName("classes")]
		public string[] Classes { get; set; } = [];
		[JsonPropertyName("classes_rename")]
		public Dictionary<string, string> ClassesRename { get; set; } = new();
		[JsonPropertyName("excludes")]
		public Dictionary<string, ExcludedObjectCollection> Excludes { get; set; } = new();
	}
}
