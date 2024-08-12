using CppAst;

namespace BindingGenerator.Models;

public class HashMapDefinition(Module module, ModuleItem? moduleItem, TypeDefinition type) : TypeDefinition(module, moduleItem)
{
    public TypeDefinition Type => type;
    public override string GetUniqueName()
    {
        return $"HashMap<{Type.GetUniqueName()}>";
    }

    public static bool IsHashMap(CppType type)
    {
        return string.Equals(type.GetDisplayName(), "hashmap");
    }
}