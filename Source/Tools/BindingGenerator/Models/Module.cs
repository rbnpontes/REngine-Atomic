﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace BindingGenerator.Models
{
	internal class Module
	{
		[JsonPropertyName("name")]
		public string Name { get; set; } = string.Empty;
		[JsonPropertyName("namespace")]
		public string Namespace { get; set; } = string.Empty;
		[JsonPropertyName("modules")]
		public string[] Modules { get; set; } = [];
		[JsonPropertyName("moduleExclude")]
		public Dictionary<string, string[]> ModuleExclude { get; set; } = new();
	}
}
