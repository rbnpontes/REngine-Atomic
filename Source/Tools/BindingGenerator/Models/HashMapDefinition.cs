using CppAst;

namespace BindingGenerator.Models;

public class HashMapDefinition(TypeDefinition type) : TypeDefinition(TypeDefKind.HashMap)
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