namespace BindingGenerator.Models;

public class ReferenceTypeDefinition(TypeDefinition type) : TypeDefinition(TypeDefKind.Ref)
{
    public TypeDefinition Type => type;
    public override string GetUniqueName()
    {
        return type.GetUniqueName() + "&";
    }
}