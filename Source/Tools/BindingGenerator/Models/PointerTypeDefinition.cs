namespace BindingGenerator.Models;

public class PointerTypeDefinition(TypeDefinition type) : TypeDefinition(TypeDefKind.Pointer)
{
    public TypeDefinition Type => type;
    public override string GetUniqueName()
    {
        return type.GetUniqueName() + "*";
    }
}