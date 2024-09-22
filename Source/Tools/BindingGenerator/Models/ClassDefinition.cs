namespace BindingGenerator.Models;

public class ClassDefinition(NamespaceDefinition @namespace) : TypeDefinition(TypeDefKind.Class)
{
    public bool IsAbstract { get; set; }
    public NamespaceDefinition Namespace { get; } = @namespace;
    public ClassMethodDefinition[] Methods { get; set; } = [];
    public ConstructorMethodDefinition[] Constructors { get; set; } = [];
    public ClassFieldTypeDefinition[] Fields { get; set; } = [];
    public ClassDefinition? Parent { get; set; }

    public override string GetUniqueName()
    {
        return Namespace.GetUniqueName() + "::" + Name;
    }
}