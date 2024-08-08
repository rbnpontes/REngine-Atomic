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
    VariantMap
}
public class PrimitiveTypeDefinition(Module module, ModuleItem? moduleItem, PrimitiveKind kind) : TypeDefinition(module, moduleItem)
{
    public PrimitiveKind Kind => kind;
    public override string GetUniqueName()
    {
        return Enum.GetName(typeof(PrimitiveKind), kind) ?? string.Empty;
    }
}