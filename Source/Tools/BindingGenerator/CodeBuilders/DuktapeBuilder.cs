﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BindingGenerator.Models;
using BindingGenerator.Utils;

namespace BindingGenerator.CodeBuilders
{
	public class DuktapeBuilder : CppBuilder
	{
		public DuktapeBuilder() : base()
		{
			SetTopComment(BindingFileUtils.GeneratedFileComment);
			Include("<Duktape/duktape.h>");
		}

		public DuktapeBuilder Pop()
		{
			Line("duk_pop(ctx);");
			return this;
		}
		public DuktapeBuilder PushObject()
		{
			Line("duk_push_object(ctx);");
			return this;
		}

		public DuktapeBuilder PushInt(int value)
		{
			return PushInt(value.ToString());
		}

		public DuktapeBuilder PushInt(string accessor)
		{
			Line($"duk_push_int(ctx, {accessor});");
			return this;
		}
		public DuktapeBuilder PutPropStringLiteral(int objectIdx, string propName)
		{
			return PutPropStringLiteral(objectIdx.ToString(), propName);
		}

		public DuktapeBuilder PutPropStringLiteral(string accessor, string propName)
		{
			return PutPropString(accessor, $"\"{propName}\"");
		}

		public DuktapeBuilder PutPropString(int objectIdx, string propNameAccessor)
		{
			PutPropString(objectIdx.ToString(), propNameAccessor);
			return this;
		}

		public DuktapeBuilder PutPropString(string accessor, string propNameAccessor)
		{
			Line($"duk_put_prop_string(ctx, {accessor}, {propNameAccessor});");
			return this;
		}

		public DuktapeBuilder PutGlobalStringLiteral(string propName)
		{
			return PutGlobalString($"\"{propName}\"");
		}
		public DuktapeBuilder PutGlobalString(string propNameAccessor)
		{
			Line($"duk_put_global_string(ctx, {propNameAccessor});");
			return this;
		}

		public DuktapeBuilder PushFunction(string accessor, int argsCount)
		{
			return PushFunction(accessor, argsCount.ToString());
		}
		public DuktapeBuilder PushFunction(string accessor, string argsAccessor)
		{
			Line($"duk_push_c_function(ctx, {accessor}, {argsAccessor});");
			return this;
		}

		public DuktapeBuilder AssertHeap()
		{
			Line($"assert_heap(ctx);");
			return this;
		}
		public DuktapeBuilder UsingNamespace(NamespaceDefinition ns)
		{
			List<NamespaceDefinition> namespaces = new();
			var currNs = ns;
			while (currNs is not null)
			{
				namespaces.Add(currNs);
				currNs = currNs.Owner;
			}

			for (var i = 0; i < namespaces.Count; ++i)
			{
				if (i == 0)
				{
					Line("using_global_namespace(ctx);");
					continue;
				}

				Line($"using_namespace(ctx, {namespaces[i - (namespaces.Count - 1)].Name});");
			}
			return this;
		}
		public static DuktapeBuilder From(CppBuilder builder)
		{
			var duktapeBuilder = new DuktapeBuilder();
			CppBuilder.Assign(builder, duktapeBuilder);
			return duktapeBuilder;
		}
	}
}
