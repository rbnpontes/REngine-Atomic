namespace BindingGenerator.Models;

public class ClassDefinition(Module module, ModuleItem moduleItem, NamespaceDefinition @namespace) : TypeDefinition(module, moduleItem)
{
    public bool IsAbstract { get; set; }
    public NamespaceDefinition Namespace { get; } = @namespace;
    public override string GetUniqueName()
    {
        return Namespace.GetUniqueName() + "::" + Name;
    }
}