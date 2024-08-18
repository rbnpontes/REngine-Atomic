using CppAst;

namespace BindingGenerator.Models;

public class SmartPointerTypeDefinition(TypeDefinition type, bool isWeak = false) : TypeDefinition(TypeDefKind.SmartPtr)
{
    public TypeDefinition Type => type;
    public bool IsWeak { get; set; } = isWeak;
    
    public override string GetUniqueName()
    {
        if (IsWeak)
            return $"WeakPtr<{Type.GetUniqueName()}>";
        return $"SharedPtr<{Type.GetUniqueName()}>";
    }

    public static bool IsSmartPointer(CppType type)
    {
        return IsWeakPtr(type) || IsSharedPtr(type);
    }
    public static bool IsWeakPtr(CppType type)
    {
        return string.Equals(type.GetDisplayName().ToLowerInvariant(), "weakptr");
    }

    public static bool IsSharedPtr(CppType type)
    {
        return string.Equals(type.GetDisplayName().ToLowerInvariant(), "sharedptr");
    }
}