namespace BindingGenerator.Models;

public class StructDefinition(Module module, ModuleItem moduleItem, NamespaceDefinition @namespace) : TypeDefinition(module, moduleItem)
{
    public NamespaceDefinition Namespace { get; } = @namespace;
    public StructMethodDefinition[] Methods { get; set; } = [];

    public override string GetUniqueName()
    {
        return Namespace.GetUniqueName() + "::" + Name;
    }
}