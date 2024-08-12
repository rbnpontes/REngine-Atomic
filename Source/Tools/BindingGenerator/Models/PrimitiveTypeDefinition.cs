using CppAst;

namespace BindingGenerator.Models;

public enum PrimitiveKind
{
    Void,
    Bool,
    WideChar,
    Char,
    Short,
    Int,
    Long,
    LongLong,
    UChar,
    UShort,
    UInt,
    ULong,
    ULongLong,
    Float,
    Double,
    LongDouble,
    String,
    VariantMap,
    StringHash,
    Variant,
    Unknow // Must be the last item
}
public class PrimitiveTypeDefinition(PrimitiveKind kind) : TypeDefinition(TypeDefKind.Primitive)
{
    public PrimitiveKind Kind => kind;
    public override string GetUniqueName()
    {
        return Enum.GetName(typeof(PrimitiveKind), kind) ?? string.Empty;
    }

    public static bool IsString(CppType type)
    {
        return string.Equals(type.GetDisplayName().ToLowerInvariant(), "string");
    }
    public static bool IsVariant(CppType type)
    {
        return string.Equals(type.GetDisplayName().ToLowerInvariant(), "variant");
    }
    public static bool IsStringHash(CppType type)
    {
        return string.Equals(type.GetDisplayName().ToLowerInvariant(), "stringhash");
    }
}