using System;
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

		public DuktapeBuilder PushCurrentFunction()
		{
			Line("duk_push_current_function(ctx);");
			return this;
		}

		public DuktapeBuilder PushPointer(string accessor)
		{
			Line($"duk_push_pointer(ctx, {accessor});");
			return this;
		}

		public DuktapeBuilder PutProp(int objIndex)
		{
			return PutProp(objIndex.ToString());
		}
		public DuktapeBuilder PutProp(string objAccessor)
		{
			Line($"duk_put_prop(ctx, {objAccessor});");
			return this;
		}
		
		public DuktapeBuilder AssertHeap()
		{
			Line($"assert_heap(ctx);");
			return this;
		}

		public DuktapeBuilder AssertHeap(int num)
		{
			return AssertHeap(num.ToString());
		}

		public DuktapeBuilder AssertHeap(string accessorNum)
		{
			Line($"assert_heap_num(ctx, {accessorNum});");
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

				Line($"using_namespace(ctx, {namespaces[(namespaces.Count - 1) - i].Name});");
			}
			return this;
		}

		public DuktapeBuilder PushThis()
		{
			Line("duk_push_this(ctx);");
			return this;
		}
		
		public DuktapeBuilder TypeGet(StructDefinition @struct)
		{
			return TypeGet(@struct.Namespace, @struct.Name);
		}

		private DuktapeBuilder TypeGet(NamespaceDefinition ns, string typeName)
		{
			var namespaces = new List<string>();
			var currNs = ns;
			while (currNs is not null)
			{
				if(!string.IsNullOrEmpty(currNs.Name))
					namespaces.Add(currNs.Name);
				currNs = currNs.Owner;
			}

			StringBuilder requireCall = new();
			requireCall.Append($"type_get(ctx, \"{typeName}\", {namespaces.Count}, ");
			for (var i = 0; i < namespaces.Count; ++i)
			{
				var idx = (namespaces.Count - 1) - i;
				requireCall.Append('\"');
				requireCall.Append(namespaces[idx]);
				requireCall.Append('\"');

				if (i < namespaces.Count - 1)
					requireCall.Append(", ");
			}

			requireCall.Append(");");
			Line(requireCall.ToString());
			return this;
		}

		public DuktapeBuilder NativeStorePointer(int objIdx, string instanceAccessor)
		{
			return NativeStorePointer(objIdx.ToString(), instanceAccessor);
		}
		public DuktapeBuilder NativeStorePointer(string objAccessor, string instanceAccessor)
		{
			Line($"native_store_pointer(ctx, {objAccessor}, {instanceAccessor});");
			return this;
		}

		public DuktapeBuilder NativeGetPointer(string varAccessor, int objIndex)
		{
			return NativeGetPointer(varAccessor, objIndex.ToString());
		}
		public DuktapeBuilder NativeGetPointer(string varAccessor, string objAccessor)
		{
			Line($"{varAccessor} = native_get_pointer(ctx, {objAccessor});");
			return this;
		}
		
		public static DuktapeBuilder From(CppBuilder builder)
		{
			var duktapeBuilder = new DuktapeBuilder();
			CppBuilder.Assign(builder, duktapeBuilder);
			return duktapeBuilder;
		}

		public DuktapeBuilder SetFinalizer(int functionIdx)
		{
			return SetFinalizer(functionIdx.ToString());
		}
		public DuktapeBuilder SetFinalizer(string functionAccessor)
		{
			Line($"duk_set_finalizer(ctx, {functionAccessor});");
			return this;
		}
		public DuktapeBuilder GetPropStringLiteral(int objIdx, string propName)
		{
			return GetPropString(objIdx, $"\"{propName}\"");
		}
		public DuktapeBuilder GetPropStringLiteral(string objAccessor, string propName)
		{
			return GetPropString(objAccessor, $"\"{propName}\"");
		}
		public DuktapeBuilder GetPropString(int objIdx, string propNameAccessor)
		{
			return GetPropString(objIdx.ToString(), propNameAccessor);
		}
		public DuktapeBuilder GetPropString(string accessorIdx, string propNameAccessor)
		{
			Line($"duk_get_prop_string(ctx, {accessorIdx}, {propNameAccessor});");
			return this;
		}

		public DuktapeBuilder New(int argsCount)
		{
			return New(argsCount.ToString());
		}

		public DuktapeBuilder New(string accessor)
		{
			Line($"duk_new(ctx, {accessor});");
			return this;
		}
	}
}
