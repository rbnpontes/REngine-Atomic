namespace BindingGenerator.Models;

public class StructDefinition(NamespaceDefinition @namespace) : TypeDefinition(TypeDefKind.Struct)
{
    public NamespaceDefinition Namespace { get; } = @namespace;
    public StructMethodDefinition[] Methods { get; set; } = [];

    public override string GetUniqueName()
    {
        return Namespace.GetUniqueName() + "::" + Name;
    }
}